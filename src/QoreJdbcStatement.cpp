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

bool QoreJdbcStatement::exec(Env& env, const QoreString& qstr, const QoreListNode* args, ExceptionSink* xsink) {
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

    // no exception handling needed; calls must be wrapped in a try/catch block
    std::vector<jvalue> jargs(1);
    LocalReference<jstring> jurl = env.newString(str->c_str());
    jargs[0].l = jurl;

    stmt = env.callObjectMethod(conn->getConnectionObject(), Globals::methodConnectionPrepareStatement, &jargs[0])
        .makeGlobal();

    if (hasArrays(args)) {
        if (bindInternArray(env, *params, xsink)) {
            return -1;
        }
    } else {
        if (bindIntern(env, *params, xsink)) {
            return -1;
        }
    }

    return env.callBooleanMethod(stmt, Globals::methodPreparedStatementExecute, nullptr);
}

int QoreJdbcStatement::describeResultSet(Env& env, ExceptionSink* xsink, LocalReference<jobject>& rs) {
    LocalReference<jobject> info = env.callObjectMethod(rs, Globals::methodResultSetGetMetaData,
        nullptr);
    assert(info);
    jint count = env.callIntMethod(info, Globals::methodResultSetMetaDataGetColumnCount, nullptr);
    printd(0, "QoreJdbcStatement::describeResultSet() column count: %d\n", count);
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
        LocalReference<jstring> cname = env.callObjectMethod(info, Globals::methodResultSetMetaDataGetColumnName,
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
                QoreStringMaker tmp("%s_%d", name, num);
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

QoreHashNode* QoreJdbcStatement::getOutputHash(Env& env, ExceptionSink* xsink, bool emptyHashIfNothing, int maxRows) {
    LocalReference<jobject> rs = env.callObjectMethod(stmt, Globals::methodPreparedStatementGetResultSet, nullptr);
    if (!rs) {
        return nullptr;
    }

    if (describeResultSet(env, xsink, rs)) {
        return nullptr;
    }

    ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), xsink);
    for (auto& i : cvec) {
        HashAssignmentHelper hah(**rv, i.qname.c_str());
        hah.assign(new QoreListNode(autoTypeInfo), xsink);
        assert(!*xsink);
    }

    while (true) {
        // get next row
        if (!env.callBooleanMethod(rs, Globals::methodResultSetNext, nullptr)) {
            break;
        }

        //! get column data
        jint c = 0;
        HashIterator i(**rv);
        while (i.next()) {
            QoreJdbcColumn& col = cvec[c];
            ValueHolder val(getColumnValue(env, rs, c + 1, col, xsink), xsink);
            if (*xsink) {
                return nullptr;
            }

            i.get().get<QoreListNode>()->push(val.release(), xsink);
            assert(!*xsink);
            ++c;
        }
    }

    return rv.release();
}

QoreListNode* QoreJdbcStatement::getOutputList(Env& env, ExceptionSink* xsink, int maxRows) {
    LocalReference<jobject> rs = env.callObjectMethod(stmt, Globals::methodPreparedStatementGetResultSet, nullptr);
    if (!rs) {
        return nullptr;
    }

    if (describeResultSet(env, xsink, rs)) {
        return nullptr;
    }

    ReferenceHolder<QoreListNode> l(new QoreListNode(autoTypeInfo), xsink);

    int rowCount = 0;
    while (true) {
        // make sure there is at least one row
        if (!env.callBooleanMethod(rs, Globals::methodResultSetNext, nullptr)) {
            // if not, return NOTHING
            break;
        }

        ReferenceHolder<QoreHashNode> h(getSingleRowIntern(env, rs, xsink), xsink);
        if (!h) {
            break;
        }
        l->push(h.release(), xsink);
        ++rowCount;
        if (maxRows > 0 && rowCount == maxRows) {
            break;
        }
    }

    return l.release();
}

QoreHashNode* QoreJdbcStatement::getSingleRow(Env& env, ExceptionSink* xsink) {
    LocalReference<jobject> rs = env.callObjectMethod(stmt, Globals::methodPreparedStatementGetResultSet, nullptr);
    if (!rs) {
        return nullptr;
    }

    // make sure there is at least one row
    if (!env.callBooleanMethod(rs, Globals::methodResultSetNext, nullptr)) {
        // if not, return NOTHING
        return nullptr;
    }

    if (describeResultSet(env, xsink, rs)) {
        return nullptr;
    }

    ReferenceHolder<QoreHashNode> rv(getSingleRowIntern(env, rs, xsink), xsink);

    // make sure there's not another row
    if (env.callBooleanMethod(rs, Globals::methodResultSetNext, nullptr)) {
        xsink->raiseException("JDBC-SELECT-ROW-ERROR", "SQL passed to selectRow() returned more than 1 row");
        return nullptr;
    }

    return rv.release();
}

QoreHashNode* QoreJdbcStatement::getSingleRowIntern(Env& env, LocalReference<jobject>& rs, ExceptionSink* xsink) {
    // get the row to return
    ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), xsink);
    jint c = 0;
    for (auto& col : cvec) {
        ValueHolder val(getColumnValue(env, rs, c + 1, col, xsink), xsink);
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

QoreValue QoreJdbcStatement::getColumnValue(Env& env, LocalReference<jobject>& rs, int column, QoreJdbcColumn& col,
        ExceptionSink* xsink) {
    // get column value for this row
    jvalue jarg;
    jarg.i = column;
    LocalReference<jobject> val = env.callObjectMethod(rs, Globals::methodResultSetGetObject, &jarg);
    if (!val) {
        return &Null;
    }
    // return Qore value
    ValueHolder rv(JavaToQore::convertToQore(val.release(), conn->getProgram(), false), xsink);
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
    params = new QoreListNode;
    paramCountInSql = 0;

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
                ++paramCountInSql;
                if (v) {
                    params->push(v.refSelf(), xsink);
                }
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
    for (unsigned int i = 0; i < count; ++i) {
        QoreValue arg = args->retrieveEntry(i);
        if (bindParamSingleValue(env, i + 1, arg, xsink)) {
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
            jargs[1].j = arg.getAsBigInt();
            env.callVoidMethod(stmt, Globals::methodPreparedStatementSetLong, &jargs[0]);
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
            int64 epoch_us = dt->getEpochMicrosecondsUTC();
            jlong ms = epoch_us / 1000;
            // set milliseconds
            jarg.j = ms;
            LocalReference<jobject> ts = env.newObject(Globals::classTimestamp, Globals::ctorTimestamp, &jarg);
            // set nanoseconds
            jint ns = epoch_us - (ms * 1000);
            jarg.i = ns;
            env.callVoidMethod(Globals::classTimestamp, Globals::methodTimestampSetNanos, &jarg);
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

/*
int QoreJdbcStatement::bindParamArraySingleValue(Env& env, int column, QoreValue arg, ExceptionSink* xsink) {
    if (arg.isNullOrNothing()) {
        char* array = arrayHolder.getNullArray(xsink);
        if (!array)
            return -1;
        indArray = arrayHolder.getNullIndArray(xsink);
        if (!indArray)
            return -1;

        ret = SQLBindParameter(stmt, column, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, 0, 0, array, 0, indArray);
        if (!SQL_SUCCEEDED(ret)) { // error
            std::string s("failed binding NULL single value parameter for column #%d");
            ODBCErrorHelper::extractDiag(SQL_HANDLE_STMT, stmt, s);
            xsink->raiseException("JDBC-BIND-ERROR", s.c_str(), column);
            return -1;
        }
        return 0;
    }
}
*/

int QoreJdbcStatement::bindInternArray(Env& env, const QoreListNode* args, ExceptionSink* xsink) {
    // Check that enough parameters were passed for binding.
    size_t count = args ? args->size() : 0;
    if (paramCountInSql != count) {
        xsink->raiseException("JDBC-BIND-ERROR",
            "mismatch between the parameter list size and number of parameters in the SQL command; %lu "
            "required, %lu passed", paramCountInSql, args->size());
        return -1;
    }

    for (unsigned int i = 0; i < count; ++i) {
        QoreValue arg = args->retrieveEntry(i);

        if (arg.getType() != NT_LIST) {
            if (bindParamSingleValue(env, i + 1, arg, xsink)) {
                return -1;
            }
            continue;
        }

        assert(false);
    }

    return 0;
}

}
