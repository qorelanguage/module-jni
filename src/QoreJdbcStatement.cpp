/* -*- indent-tabs-mode: nil -*- */
/*
    QoreJdbcStatement.cpp

    Qore Programming Language JNI Module

    Copyright (C) 2016 - 2023 Qore Technologies, s.r.o.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "QoreJdbcStatement.h"
#include "Globals.h"
#include "QoreToJava.h"
#include "JavaToQore.h"

#include <set>

namespace jni {

QoreJdbcColumn::QoreJdbcColumn(std::string&& name, std::string&& qname, jint ctype) : name(name), qname(qname),
        strip(ctype == Globals::typeChar) {
}

QoreJdbcStatement::~QoreJdbcStatement() {
    if (stmt) {
        Env env;
        close(env);
    }
}

bool QoreJdbcStatement::exec(Env& env, ExceptionSink* xsink, const QoreString& qstr, const QoreListNode* args) {
    assert(!stmt);

    // Convert string to required character encoding.
    TempEncodingHelper str(qstr, QCS_UTF8, xsink);
    if (*xsink) {
        return false;
    }
    str.makeTemp();
    if (parse(const_cast<QoreString*>(*str), args, xsink)) {
        return false;
    }

    prepareAndBindStatement(env, xsink, **str);

    return execIntern(env, **str, xsink);
}

void QoreJdbcStatement::prepareAndBindStatement(Env& env, ExceptionSink* xsink, const QoreString& str) {
    prepareStatement(env, str);
    bindQueryArguments(env, xsink);
}

void QoreJdbcStatement::prepareStatement(Env& env, const QoreString& str) {
    assert(!stmt);
    // no exception handling needed; calls must be wrapped in a try/catch block
    std::vector<jvalue> jargs(1);
    LocalReference<jstring> jstr = env.newString(str.c_str());
    jargs[0].l = jstr;

    stmt = env.callObjectMethod(conn->getConnectionObject(), Globals::methodConnectionPrepareStatement, &jargs[0])
        .makeGlobal();
}

int QoreJdbcStatement::bindQueryArguments(Env& env, ExceptionSink* xsink) {
    if (hasArrayBind()) {
        if (bindInternArray(env, *params, xsink)) {
            return -1;
        }
    } else if (bindIntern(env, *params, xsink)) {
        return -1;
    }
    return 0;
}

bool QoreJdbcStatement::execIntern(Env& env, const QoreString& qstr, ExceptionSink* xsink) {
    do {
        // check for a lost connection
        try {
            if (do_batch_execute) {
                // ignore return value
                env.callObjectMethod(stmt, Globals::methodPreparedStatementExecuteBatch, nullptr);
                return false;
            }
            return env.callBooleanMethod(stmt, Globals::methodPreparedStatementExecute, nullptr);
        } catch (JavaException& e) {
            LocalReference<jthrowable> throwable = e.save();
            assert(throwable);
            if (env.isInstanceOf(throwable, Globals::classSQLException)) {
                // check if connection is still valid; 1 second timeout
                jvalue jarg;
                jarg.i = 1;
                bool connected = env.callBooleanMethod(conn->getConnectionObject(), Globals::methodConnectionIsValid,
                    &jarg);
                //printd(5, "QoreJdbcStatement::execIntern() connected: %d\n", connected);
                if (!connected && !reconnectLostConnection(env, xsink)) {
                    assert(!*xsink);
                    // repeat statement execution after reconnection when not in a transaction
                    prepareAndBindStatement(env, xsink, qstr);
                    continue;
                }
            }
            e.restore(throwable.release());
            throw;
        }
    } while (false);
    return false;
}

void QoreJdbcStatement::close(Env& env_obj) {
    // not using the Env wrapper because we don't want any C++ exceptions here
    JNIEnv* env = *env_obj;
    bool active_java_exception = env->ExceptionCheck();

    if (rs) {
        env->CallVoidMethodA(rs, Globals::methodResultSetClose, nullptr);
        rs = nullptr;
    }
    if (stmt) {
        env->CallVoidMethodA(stmt, Globals::methodPreparedStatementClose, nullptr);
        stmt = nullptr;
    }
    if (!active_java_exception && env->ExceptionCheck()) {
        throw new JavaException;
    }
}

void QoreJdbcStatement::reset(Env& env) {
    close(env);

    bind_size = 0;
    array_bind_size = 0;
    do_batch_execute = false;
    params = nullptr;
    cvec.clear();
}

int QoreJdbcStatement::reconnectLostConnection(Env& env, ExceptionSink* xsink) {
    if (conn->getDatasource()->activeTransaction()) {
        printd(0, "QoreJdbcStatement::reconnectLostConnection() connection lost while in a transaction\n");
        xsink->raiseException("ODBC-TRANSACTION-ERROR", "connection to database server lost while in a "
            "transaction; transaction has been lost");
    }

    // Reset current statement state while the driver-specific context data is still present
    close(env);

    // Free and reset statement states for all active statements while the driver-specific context data is still
    // present
    conn->getDatasource()->connectionLost(xsink);

    // Disconnect first
    conn->close(env);

    // try to reconnect
    if (conn->reconnect(env, xsink)) {
        // Free state completely.
        reset(env);

        // Reconnect failed; marking connection as closed
        // The following call will close any open statements and then the datasource
        conn->getDatasource()->connectionAborted();
        return -1;
    }

    // Don't execute again if any exceptions have occured
    if (*xsink) {
        // Close all statements and remove private data but leave datasource open
        conn->getDatasource()->connectionRecovered(xsink);
        return -1;
    }

    return 0;
}

int QoreJdbcStatement::describeResultSet(Env& env, ExceptionSink* xsink) {
    assert(rs);
    LocalReference<jobject> info = env.callObjectMethod(rs, Globals::methodResultSetGetMetaData,
        nullptr);
    assert(info);
    jint count = env.callIntMethod(info, Globals::methodResultSetMetaDataGetColumnCount, nullptr);
    //printd(5, "QoreJdbcStatement::describeResultSet() column count: %d\n", count);
    if (!count) {
        return -1;
    }

    assert(cvec.empty());
    cvec.reserve(count);

    // detect duplicate column names in the output
    typedef std::set<std::string> strset_t;
    strset_t strset;

    ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), xsink);
    for (jint i = 0; i < count; ++i) {
        jvalue jarg;
        jarg.i = i + 1;
        // NOTE: if we use ResultSetMetaData.getColumnName() it will ignore column aliases
        LocalReference<jstring> cname = env.callObjectMethod(info, Globals::methodResultSetMetaDataGetColumnLabel,
            &jarg).as<jstring>();
        Env::GetStringUtfChars name(env, cname);

        // convert to lower case
        QoreString qname(name.c_str());
        qname.tolwr();

        std::string unique_qname;
        strset_t::iterator it = strset.find(qname.c_str());
        if (it == strset.end()) {
            unique_qname = qname.c_str();
        } else {
            // Find a unique column name.
            unsigned num = 1;
            while (true) {
                QoreStringMaker tmp("%s_%d", name.c_str(), num);
                it = strset.find(tmp.c_str());
                if (it == strset.end()) {
                    unique_qname = tmp.c_str();
                    break;
                }
                ++num;
                continue;
            }
        }
        strset.insert(it, unique_qname);

        // get column type
        jint ctype = env.callIntMethod(info, Globals::methodResultSetMetaDataGetColumnType, &jarg);
        cvec.emplace_back(QoreJdbcColumn(qname.c_str(), std::move(unique_qname), ctype));
    }

    return 0;
}

bool QoreJdbcStatement::next(Env& env) {
    assert(rs);
    return env.callBooleanMethod(rs, Globals::methodResultSetNext, nullptr);
}

int QoreJdbcStatement::acquireResultSet(Env& env, ExceptionSink* xsink) {
    assert(!rs);
    rs = env.callObjectMethod(stmt, Globals::methodPreparedStatementGetResultSet, nullptr);
    if (!rs) {
        xsink->raiseException("JDBC-RESULTSET-ERROR", "no result set available from query");
        return -1;
    }

    return 0;
}

QoreHashNode* QoreJdbcStatement::getOutputHash(Env& env, ExceptionSink* xsink, bool empty_hash_if_nothing,
        int max_rows) {
    if (acquireResultSet(env, xsink) || describeResultSet(env, xsink)) {
        return nullptr;
    }

    return getOutputHashIntern(env, xsink, empty_hash_if_nothing, max_rows);
}

void QoreJdbcStatement::populateOutputHash(QoreHashNode& h, ExceptionSink* xsink) {
    for (auto& i : cvec) {
        h.setKeyValue(i.qname.c_str(), new QoreListNode(autoTypeInfo), xsink);
        assert(!*xsink);
    }
}

typedef std::vector<QoreListNode*> lvec_t;

class QoreListArray {
public:
    DLLLOCAL QoreListArray(ExceptionSink* xsink, cvec_t& cvec) : xsink(xsink), cvec(cvec) {
    }

    DLLLOCAL ~QoreListArray() {
        for (auto& i : l) {
            i->deref(xsink);
        }
        l.clear();
    }

    DLLLOCAL void populate() {
        assert(l.empty());
        for (size_t i = 0, e = cvec.size(); i < e; ++i) {
            l.push_back(new QoreListNode(autoTypeInfo));
        }
    }

    DLLLOCAL lvec_t& get() {
        return l;
    }

    DLLLOCAL QoreHashNode* getHash() {
        ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), xsink);
        if (!l.empty()) {
            for (size_t i = 0, e = cvec.size(); i < e; ++i) {
                rv->setKeyValue(cvec[i].qname.c_str(), l[i], xsink);
            }
            l.clear();
        }
        return rv.release();
    }

protected:
    lvec_t l;
    ExceptionSink* xsink;
    cvec_t& cvec;
};

QoreHashNode* QoreJdbcStatement::getOutputHashIntern(Env& env, ExceptionSink* xsink, bool empty_hash_if_nothing,
        int max_rows) {
    // we have to populate the lists and then assign the hash in case we have Java objects returned for DB values
    QoreListArray l(xsink, cvec);

    size_t row_count = 0;
    while (true) {
        // get next row
        if (!next(env)) {
            break;
        }

        if (!row_count) {
            l.populate();
        }

        //! get column data
        for (jint c = 0, e = (jint)cvec.size(); c < e; ++c) {
            QoreJdbcColumn& col = cvec[c];
            ValueHolder val(getColumnValue(env, c + 1, col, xsink), xsink);
            if (*xsink) {
                return nullptr;
            }

            l.get()[c]->push(val.release(), xsink);
            assert(!*xsink);
        }
        ++row_count;
    }

    if (!row_count && !empty_hash_if_nothing) {
        l.populate();
    }
    return l.getHash();
}

QoreListNode* QoreJdbcStatement::getOutputList(Env& env, ExceptionSink* xsink, int max_rows) {
    if (acquireResultSet(env, xsink) || describeResultSet(env, xsink)) {
        return nullptr;
    }

    return getOutputListIntern(env, xsink, max_rows);
}

QoreListNode* QoreJdbcStatement::getOutputListIntern(Env& env, ExceptionSink* xsink, int max_rows) {
    ReferenceHolder<QoreListNode> l(new QoreListNode(autoTypeInfo), xsink);

    int rowCount = 0;
    while (true) {
        // make sure there is at least one row
        if (!next(env)) {
            // if not, return NOTHING
            break;
        }

        ReferenceHolder<QoreHashNode> h(getSingleRowIntern(env, xsink), xsink);
        if (!h) {
            break;
        }
        l->push(h.release(), xsink);
        ++rowCount;
        if (max_rows > 0 && rowCount == max_rows) {
            break;
        }
    }

    return l.release();
}

QoreHashNode* QoreJdbcStatement::getSingleRow(Env& env, ExceptionSink* xsink) {
    assert(!rs);
    rs = env.callObjectMethod(stmt, Globals::methodPreparedStatementGetResultSet, nullptr);
    if (!rs) {
        return nullptr;
    }

    // make sure there is at least one row
    if (!next(env)) {
        // if not, return NOTHING
        return nullptr;
    }

    if (describeResultSet(env, xsink)) {
        return nullptr;
    }

    ReferenceHolder<QoreHashNode> rv(getSingleRowIntern(env, xsink), xsink);

    // make sure there's not another row
    if (next(env)) {
        xsink->raiseException("JDBC-SELECT-ROW-ERROR", "SQL passed to selectRow() returned more than 1 row");
        return nullptr;
    }

    return rv.release();
}

QoreHashNode* QoreJdbcStatement::getSingleRowIntern(Env& env, ExceptionSink* xsink) {
    assert(rs);
    // get the row to return
    ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), xsink);
    jint c = 0;
    for (auto& col : cvec) {
        ValueHolder val(getColumnValue(env, c + 1, col, xsink), xsink);
        if (*xsink) {
            return nullptr;
        }

        HashAssignmentHelper hah(**rv, col.qname.c_str());
        hah.assign(val.release(), xsink);
        assert(!*xsink);
        ++c;
    }

    return rv.release();
}

QoreValue QoreJdbcStatement::getColumnValue(Env& env, int column, QoreJdbcColumn& col, ExceptionSink* xsink) {
    // get column value for this row
    jvalue jarg;
    jarg.i = column;
    LocalReference<jobject> val = env.callObjectMethod(rs, Globals::methodResultSetGetObject, &jarg);
    if (!val) {
        return &Null;
    }
    // return Qore value
    ValueHolder rv(JavaToQore::convertToQore(val.release(), conn->getProgram(), false, conn->getNumericOption()),
        xsink);
    // strip trailing spaces in CHAR columns if necessary
    if (col.strip && rv->getType() == NT_STRING) {
        rv->get<QoreStringNode>()->trim_trailing(' ');
    }
    return rv.release();
}

int QoreJdbcStatement::rowsAffected(Env& env) {
    return env.callIntMethod(stmt, Globals::methodPreparedStatementGetUpdateCount, nullptr);
}

int QoreJdbcStatement::parse(QoreString* str, const QoreListNode* args, ExceptionSink* xsink) {
    char quote = 0;
    const char *p = str->c_str();
    QoreString tmp;
    int index = 0;
    SQLCommentType comment = ESCT_NONE;
    params = nullptr;
    bind_size = 0;

    while (*p) {
        if (!quote) {
            if (comment == ESCT_NONE) {
                if ((*p) == '-' && (*(p+1)) == '-') {
                    comment = ESCT_LINE;
                    p += 2;
                    continue;
                }

                if ((*p) == '/' && (*(p+1)) == '*') {
                    comment = ESCT_BLOCK;
                    p += 2;
                    continue;
                }
            } else {
                if (comment == ESCT_LINE) {
                    if ((*p) == '\n' || ((*p) == '\r')) {
                        comment = ESCT_NONE;
                    }
                    ++p;
                    continue;
                }

                assert(comment == ESCT_BLOCK);
                if ((*p) == '*' && (*(p+1)) == '/') {
                    comment = ESCT_NONE;
                    p += 2;
                    continue;
                }

                ++p;
                continue;
            }

            if ((*p) == '%' && (p == str->c_str() || !isalnum(*(p-1)))) { // Found value marker.
                int offset = p - str->c_str();

                ++p;
                QoreValue v = args ? args->retrieveEntry(index++) : QoreValue();
                if ((*p) == 'd') {
                    DBI_concat_numeric(&tmp, v);
                    str->replace(offset, 2, tmp.c_str());
                    p = str->c_str() + offset + tmp.strlen();
                    tmp.clear();
                    continue;
                }
                if ((*p) == 's') {
                    if (DBI_concat_string(&tmp, v, xsink)) {
                        return -1;
                    }
                    str->replace(offset, 2, tmp.c_str());
                    p = str->c_str() + offset + tmp.strlen();
                    tmp.clear();
                    continue;
                }
                if ((*p) != 'v') {
                    xsink->raiseException("JDBC-PARSE-ERROR", "invalid value specification (expecting '%%v' or "
                        "'%%d', got %%%c)", *p);
                    return -1;
                }
                ++p;
                if (isalpha(*p)) {
                    xsink->raiseException("JDBC-PARSE-ERROR", "invalid value specification (expecting '%%v' or "
                        "'%%d', got %%v%c*)", *p);
                    return -1;
                }

                str->replace(offset, 2, "?");
                p = str->c_str() + offset + 1;
                ++bind_size;
                if (!params) {
                    params = new QoreListNode(autoTypeInfo);
                }
                params->push(v.refSelf(), xsink);
                continue;
            }

            // Allow escaping of '%' characters.
            if ((*p) == '\\' && (*(p+1) == ':' || *(p+1) == '%')) {
                str->splice(p - str->c_str(), 1, xsink);
                p += 2;
                continue;
            }
        }

        if (((*p) == '\'') || ((*p) == '\"')) {
            if (!quote) {
                quote = *p;
            } else if (quote == (*p)) {
                quote = 0;
            }
            ++p;
            continue;
        }

        ++p;
    }

    return 0;
}

size_t QoreJdbcStatement::findArraySizeOfArgs(const QoreListNode* args) const {
    size_t count = args ? args->size() : 0;
    for (unsigned int i = 0; i < count; ++i) {
        QoreValue arg = args->retrieveEntry(i);
        qore_type_t ntype = arg.getType();
        if (ntype == NT_LIST) {
            return arg.get<const QoreListNode>()->size();
        }
    }
    return 0;
}

int QoreJdbcStatement::bindIntern(Env& env, const QoreListNode* args, ExceptionSink* xsink) {
    size_t count = args ? args->size() : 0;
    // ignore excess arguments
    if (count > bind_size) {
        count = bind_size;
    }
    for (unsigned int i = 0; i < count; ++i) {
        QoreValue arg = args->retrieveEntry(i);
        if (bindParamSingleValue(env, i + 1, arg, xsink)) {
            return -1;
        }
    }
    // bind excess positions with NULL
    for (unsigned i = count; i < bind_size; ++i) {
        if (bindParamSingleValue(env, i + 1, QoreValue(), xsink)) {
            return -1;
        }
    }

    return 0;
}

int QoreJdbcStatement::bindParamSingleValue(Env& env, int column, QoreValue arg, ExceptionSink* xsink) {
    std::vector<jvalue> jargs(2);
    jargs[0].i = column;

    switch (arg.getType()) {
        case NT_NULL:
        case NT_NOTHING: {
            jargs[1].i = Globals::typeNull;
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetNull, &jargs[0]);
            break;
        }

        case NT_INT: {
            int64 i = arg.getAsBigInt();
            if (i <= 2147483647 && i >= -2147483647) {
                jargs[1].i = (int32_t)i;
                env.callVoidMethod(stmt, Globals::methodPreparedStatementSetInt, &jargs[0]);
            } else {
                jargs[1].j = i;
                env.callVoidMethod(stmt, Globals::methodPreparedStatementSetLong, &jargs[0]);
            }
            break;
        }

        case NT_FLOAT: {
            jargs[1].d = arg.getAsFloat();
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetDouble, &jargs[0]);
            break;
        }

        case NT_BOOLEAN: {
            jargs[1].z = arg.getAsBool();
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetBoolean, &jargs[0]);
            break;
        }

        case NT_STRING: {
            LocalReference<jstring> jstr = env.newString(arg.get<const QoreStringNode>()->c_str());
            jargs[1].l = jstr;
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetString, &jargs[0]);
            break;
        }

        case NT_BINARY: {
            LocalReference<jbyteArray> array = QoreToJava::makeByteArray(env, *arg.get<const BinaryNode>());
            jargs[1].l = array;
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetBytes, &jargs[0]);
            break;
        }

        case NT_DATE: {
            const DateTimeNode* dt = arg.get<const DateTimeNode>();
            jvalue jarg;
            int64 epoch_s = dt->getEpochSecondsUTC();
            int us = dt->getMicrosecond();
            // set milliseconds from seconds value
            jarg.j = epoch_s * 1000;
            LocalReference<jobject> ts = env.newObject(Globals::classTimestamp, Globals::ctorTimestamp, &jarg);
            // set nanoseconds
            jint ns = us * 1000;
            jarg.i = ns;
            env.callVoidMethod(ts, Globals::methodTimestampSetNanos, &jarg);
            // bind timestamp value
            jargs[1].l = ts;
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetTimestamp, &jargs[0]);
            break;
        }

        case NT_NUMBER: {
            LocalReference<jobject> num = QoreToJava::makeBigDecimal(env, *arg.get<const QoreNumberNode>());
            jargs[1].l = num;
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetBigDecimal, &jargs[0]);
            break;
        }

        default:
            xsink->raiseException("JDBC-BIND-ERROR", "do not know how to bind arguments of type '%s'",
                arg.getFullTypeName());
            return -1;
    }

    return 0;
}

int QoreJdbcStatement::bindInternArray(Env& env, const QoreListNode* args, ExceptionSink* xsink) {
    // Check that enough parameters were passed for binding.
    size_t count = args ? args->size() : 0;
    if (bind_size != count) {
        xsink->raiseException("JDBC-BIND-ERROR",
            "mismatch between the parameter list size and number of parameters in the SQL command; %lu "
            "required, %lu passed", bind_size, args->size());
        return -1;
    }

    return bindInternArrayBatch(env, args, xsink);
#if 0
    if (conn->areArraysSupported(env)) {
        return bindInternArrayNative(env, args, xsink);
    } else {
        return bindInternArrayBatch(env, args, xsink);
    }
#endif
}

#if 0
const char* QoreJdbcStatement::getArrayBindType(const QoreListNode* l) const {
    ConstListIterator i(l);
    while (i.next()) {
        switch (i.getValue().getType()) {
            case NT_NOTHING:
            case NT_NULL:
                break;

            case NT_INT:
                return "int";

            case NT_FLOAT:
                return "double";

            case NT_NUMBER:
                return "numeric";

            case NT_STRING:
                return "varchar";

            case NT_BOOLEAN:
                return "boolean";

            case NT_DATE:
                return "timestamp";

            case NT_BINARY:
                return "blob";

            default:
                return i.getValue().getTypeName();
        }
    }

    return "null";
}

int QoreJdbcStatement::bindInternArrayNative(Env& env, const QoreListNode* args, ExceptionSink* xsink) {
    size_t count = args ? args->size() : 0;
    for (unsigned int i = 0; i < count; ++i) {
        QoreValue arg = args->retrieveEntry(i);

        if (arg.getType() != NT_LIST) {
            if (bindParamSingleValue(env, i + 1, arg, xsink)) {
                return -1;
            }
            continue;
        }

        const char* bind_type = getArrayBindType(arg.get<const QoreListNode>());
        printd(0, "QoreJdbcStatement::bindInternArray() binding array of SQL type '%s'\n", bind_type);

        std::vector<jvalue> jargs(2);
        LocalReference<jstring> jurl = env.newString(bind_type);
        jargs[0].l = jurl;
        LocalReference<jobject> array_arg = QoreToJava::toAnyObject(env, arg, conn->getQoreJniContext());
        jargs[1].l = array_arg;
        LocalReference<jobject> array = env.callObjectMethod(conn->getConnectionObject(),
            Globals::methodConnectionCreateArrayOf, &jargs[0]);
        jargs[0].l = array;
        env.callVoidMethod(stmt, Globals::methodPreparedStatementSetArray, &jargs[0]);
    }

    return 0;
}
#endif

int QoreJdbcStatement::bindInternArrayBatch(Env& env, const QoreListNode* args, ExceptionSink* xsink) {
    do_batch_execute = true;
    size_t list_size = findArraySizeOfArgs(args);
    size_t arg_count = args ? args->size() : 0;

    for (size_t i = 0; i < list_size; ++i) {
        for (unsigned int j = 0; j < arg_count; ++j) {
            QoreValue arg = args->retrieveEntry(j);
            // get value to bind from the list if necessary
            if (arg.getType() == NT_LIST) {
                const QoreListNode* l = arg.get<const QoreListNode>();
                if (l->size() != list_size) {
                    xsink->raiseException("JDBC-BIND-ERROR", "the array size for bind argument %d (starting from 1) "
                        "is %zu which is inconsistent with the detected array size %zu.  This is an error, because "
                        "all array bind arguments must have the same array / list size.", (int)j, l->size(),
                        list_size);
                    return -1;
                }
                arg = l->retrieveEntry(i);
            }
            if (bindParamSingleValue(env, j + 1, arg, xsink)) {
                return -1;
            }
        }
        env.callVoidMethod(stmt, Globals::methodPreparedStatementAddBatch, nullptr);
    }
    return 0;
}

}
