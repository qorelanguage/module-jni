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

namespace jni {

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
    std::vector<jvalue> jargs(2);
    LocalReference<jstring> jurl = env.newString(str->c_str());
    jargs[0].l = jurl;
    jargs[1].l = nullptr;

    stmt = env.callObjectMethod(conn->getConnectionObject(), Globals::methodConnectionPrepareStatement, &jargs[0])
        .makeGlobal();

    return env.callBooleanMethod(stmt, Globals::methodPreparedStatementExecute, nullptr);
}

QoreHashNode* QoreJdbcStatement::getOutputHash(Env& env, ExceptionSink* xsink, bool emptyHashIfNothing, int maxRows) {
    return nullptr;
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

}
