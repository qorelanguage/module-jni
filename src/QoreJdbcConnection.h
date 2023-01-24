/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
    QoreJdbcConnection.h

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

#ifndef _QORE_JNI_QOREJDBCCONNECTION_H

#define _QORE_JNI_QOREJDBCCONNECTION_H

#include <qore/Qore.h>

#include "GlobalReference.h"

#include <string>

namespace jni {

class QoreJdbcConnection {
public:
    DLLLOCAL QoreJdbcConnection(Datasource* d, ExceptionSink* xsink);
    DLLLOCAL ~QoreJdbcConnection();

    DLLLOCAL jobject getConnectionObject() const {
        return connection;
    }

    //! Set an option for the connection.
    /** @param opt option name
        @param val option value to use
        @param xsink exception sink

        @return 0 for OK, -1 for error
    */
    DLLLOCAL int setOption(const char* opt, const QoreValue val, ExceptionSink* xsink);

    //! Get the current value of an option of the connection.
    /** @param opt option name

        @return option's value
    */
    DLLLOCAL QoreValue getOption(const char* opt);

    DLLLOCAL int close(ExceptionSink* xsink);

    DLLLOCAL int commit(ExceptionSink* xsink);
    DLLLOCAL int rollback(ExceptionSink* xsink);

    DLLLOCAL QoreValue getServerVersion(ExceptionSink* xsink);
    DLLLOCAL QoreValue getClientVersion(ExceptionSink* xsink);

    DLLLOCAL QoreValue select(const QoreString* qstr, const QoreListNode* args, ExceptionSink* xsink);

private:
    //! Qore datasource.
    Datasource* ds = nullptr;

    //! Connection object
    GlobalReference<jobject> connection;

    //! Classpath value
    std::string classpath;

    //! Parse options passed through the Datasource
    /** @param xsink exception sink

        @return 0 for OK, -1 for error
     */
    DLLLOCAL int parseOptions(ExceptionSink* xsink);

    QoreJdbcConnection(const QoreJdbcConnection&) = delete;
    QoreJdbcConnection& operator=(const QoreJdbcConnection&) = delete;
};

}

#endif