/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
    QoreJdbcStatement.h

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

#ifndef _QORE_JNI_QOREJDBCSTATEMENT_H

#define _QORE_JNI_QOREJDBCSTATEMENT_H

#include <qore/Qore.h>

#include "QoreJdbcConnection.h"
#include "Env.h"
#include "GlobalReference.h"

namespace jni {

class QoreJdbcStatement {
public:
    DLLLOCAL QoreJdbcStatement(ExceptionSink* xsink, QoreJdbcConnection* conn) : conn(conn), params(xsink) {
    }

    DLLLOCAL bool exec(Env& env, const QoreString& qstr, const QoreListNode* args, ExceptionSink* xsink);

    //! Return how many rows were affected by the executed statement.
    /** @return count of affected rows; -1 in case the number is not available
     */
    DLLLOCAL int rowsAffected(Env& env);

    //! Get result hash
    /** @param xsink exception sink
        @oaram emptyHashIfNothing whether to return empty hash or empty hash with column names when no rows available
        @param maxRows maximum count of rows to return; if <= 0 the count of returned rows is not limited

        @return hash of result column lists
    */
    DLLLOCAL QoreHashNode* getOutputHash(Env& env, ExceptionSink* xsink, bool emptyHashIfNothing, int maxRows = -1);

protected:
    //! Possible comment types; used in the parse() method
    enum SQLCommentType {
        ESCT_NONE = 0,
        ESCT_LINE,
        ESCT_BLOCK
    };

    //! Connection
    QoreJdbcConnection* conn;

    //! PreparedStatement object
    GlobalReference<jobject> stmt;

    //! Count of required parameters for the SQL command
    size_t paramCountInSql = 0;

    //! Parameters which will be used in the statement
    ReferenceHolder<QoreListNode> params;

    //! Parse a Qore-style SQL statement
    /** @param str Qore-style SQL statement
        @param args SQL parameters
        @param xsink exception sink

        @return 0 for OK, -1 for error
    */
    DLLLOCAL int parse(QoreString* str, const QoreListNode* args, ExceptionSink* xsink);

    //! Return size of arrays in the passed arguments
    /** @param args SQL parameters

        @return parameter array size
    */
    DLLLOCAL size_t findArraySizeOfArgs(const QoreListNode* args) const;

    //! Return whether the passed arguments have arrays
    DLLLOCAL bool hasArrays(const QoreListNode* args) const {
        return findArraySizeOfArgs(args) > 0;
    }

    //! Bind a single value argument
    /** @param column JDBC column number, starting from 1
        @param arg single value parameter
        @param xsink exception sink

        @return 0 for OK, -1 for error
    */
    DLLLOCAL int bindParamSingleValue(Env& env, int column, QoreValue arg, ExceptionSink* xsink);

    //! Bind a single value argument as an array
    /** @param column JDBC column number, starting from 1
        @param arg single value parameter
        @param xsink exception sink

        @return 0 for OK, -1 for error
    */
    DLLLOCAL int bindParamArraySingleValue(Env& env, int column, QoreValue arg, ExceptionSink* xsink);

    //! Bind a simple list of SQL parameters
    /** @param args SQL parameters
        @param xsink exception sink

        @return 0 for OK, -1 for error
    */
    DLLLOCAL int bindIntern(Env& env, const QoreListNode* args, ExceptionSink* xsink);

    //! Bind a list of arrays of SQL parameters
    /** @param args SQL parameters
        @param xsink exception sink

        @return 0 for OK, -1 for error
    */
    DLLLOCAL int bindInternArray(Env& env, const QoreListNode* args, ExceptionSink* xsink);
};

}
#endif