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

namespace jni {

class QoreJdbcConnection {
public:
    DLLLOCAL QoreJdbcConnection(Datasource* d, ExceptionSink* xsink);
    DLLLOCAL ~QoreJdbcConnection();

    DLLLOCAL jobject getConnectionObject() const {
        return connection;
    }

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

    QoreJdbcConnection(const QoreJdbcConnection&) = delete;
    QoreJdbcConnection& operator=(const QoreJdbcConnection&) = delete;
};

}

#endif