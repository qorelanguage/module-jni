/* -*- indent-tabs-mode: nil -*- */
/*
    QoreJdbcConnection.cpp

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

#include "QoreJdbcConnection.h"
#include "QoreJdbcStatement.h"
#include "LocalReference.h"
#include "Globals.h"
#include "Env.h"

#include <vector>

namespace jni {

static void set_prop(Env& env, LocalReference<jobject>& props, const char* prop, const char* val) {
    std::vector<jvalue> jargs(2);

    LocalReference<jstring> jprop = env.newString(prop);
    jargs[0].l = jprop;
    LocalReference<jstring> jval = env.newString(val);
    jargs[1].l = jval;
    LocalReference<jobject> ignore = env.callObjectMethod(props, Globals::methodPropertiesSetProperty, &jargs[0]);
}

QoreJdbcConnection::QoreJdbcConnection(Datasource* ds, ExceptionSink* xsink) : ds(ds) {
    // get URL
    const std::string& str = ds->getDBNameStr();
    if (str.empty()) {
        xsink->raiseException("JDBC-CONNECTION-ERROR", "cannot build JDBC URL without a db name");
        return;
    }
    QoreStringMaker url("jdbc:%s", str.c_str());

    Env env;
    try {
        LocalReference<jobject> props = env.newObject(Globals::classProperties, Globals::ctorProperties, nullptr);
        {
            const std::string& str = ds->getUsernameStr();
            if (!str.empty()) {
                set_prop(env, props, "user", str.c_str());
            }
        }
        {
            const std::string& str = ds->getPasswordStr();
            if (!str.empty()) {
                set_prop(env, props, "password", str.c_str());
            }
        }

        std::vector<jvalue> jargs(2);
        LocalReference<jstring> jurl = env.newString(url.c_str());
        jargs[0].l = jurl;
        jargs[1].l = props;

        connection = env.callStaticObjectMethod(Globals::classDriverManager,
            Globals::methodDriverManagerGetConnection, &jargs[0]).makeGlobal();
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
}

QoreJdbcConnection::~QoreJdbcConnection() {
    assert(!connection);
}

int QoreJdbcConnection::close(ExceptionSink* xsink) {
    if (!connection) {
        return 0;
    }
    Env env;
    try {
        env.callObjectMethod(connection, Globals::methodConnectionClose, nullptr);
        connection = nullptr;
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
    return *xsink ? -1 : 0;
}

int QoreJdbcConnection::commit(ExceptionSink* xsink) {
    assert(connection);
    Env env;
    try {
        env.callObjectMethod(connection, Globals::methodConnectionCommit, nullptr);
        connection = nullptr;
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
    return *xsink ? -1 : 0;
}

int QoreJdbcConnection::rollback(ExceptionSink* xsink) {
    assert(connection);
    Env env;
    try {
        env.callObjectMethod(connection, Globals::methodConnectionRollback, nullptr);
        connection = nullptr;
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
    return *xsink ? -1 : 0;
}

QoreValue QoreJdbcConnection::getServerVersion(ExceptionSink* xsink) {
    assert(connection);
    Env env;
    try {
        LocalReference<jobject> md = env.callObjectMethod(connection, Globals::methodConnectionGetMetaData, nullptr);
        if (!md) {
            xsink->raiseException("JDBC-METADATA-ERROR", "the connection returned no metadata");
            return QoreValue();
        }

        ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), xsink);

        LocalReference<jstring> str = env.callObjectMethod(md,
            Globals::methodDatabaseMetaDataGetDatabaseProductName, nullptr).as<jstring>();
        if (str) {
            Env::GetStringUtfChars desc(env, str);
            rv->setKeyValue("name", new QoreStringNode(desc.c_str()), xsink);
        }
        str = env.callObjectMethod(md,
            Globals::methodDatabaseMetaDataGetDatabaseProductVersion, nullptr).as<jstring>();
        if (str) {
            Env::GetStringUtfChars desc(env, str);
            rv->setKeyValue("version", new QoreStringNode(desc.c_str()), xsink);
        }
        jint i = env.callIntMethod(md,
            Globals::methodDatabaseMetaDataGetDatabaseMajorVersion, nullptr);
        rv->setKeyValue("major_version", i, xsink);
        i = env.callIntMethod(md,
            Globals::methodDatabaseMetaDataGetDatabaseMinorVersion, nullptr);
        rv->setKeyValue("minor_version", i, xsink);
        return rv.release();
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
    return *xsink ? -1 : 0;
}

QoreValue QoreJdbcConnection::getClientVersion(ExceptionSink* xsink) {
    assert(connection);
    Env env;
    try {
        LocalReference<jobject> md = env.callObjectMethod(connection, Globals::methodConnectionGetMetaData, nullptr);
        if (!md) {
            xsink->raiseException("JDBC-METADATA-ERROR", "the connection returned no metadata");
            return QoreValue();
        }

        ReferenceHolder<QoreHashNode> rv(new QoreHashNode(autoTypeInfo), xsink);

        LocalReference<jstring> str = env.callObjectMethod(md,
            Globals::methodDatabaseMetaDataGetDriverName, nullptr).as<jstring>();
        if (str) {
            Env::GetStringUtfChars desc(env, str);
            rv->setKeyValue("name", new QoreStringNode(desc.c_str()), xsink);
        }
        str = env.callObjectMethod(md,
            Globals::methodDatabaseMetaDataGetDriverVersion, nullptr).as<jstring>();
        if (str) {
            Env::GetStringUtfChars desc(env, str);
            rv->setKeyValue("version", new QoreStringNode(desc.c_str()), xsink);
        }
        jint i = env.callIntMethod(md,
            Globals::methodDatabaseMetaDataGetDriverMajorVersion, nullptr);
        rv->setKeyValue("major_version", i, xsink);
        i = env.callIntMethod(md,
            Globals::methodDatabaseMetaDataGetDriverMinorVersion, nullptr);
        rv->setKeyValue("minor_version", i, xsink);
        return rv.release();
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
    return *xsink ? -1 : 0;
}

QoreValue QoreJdbcConnection::select(const QoreString* qstr, const QoreListNode* args, ExceptionSink* xsink) {
    assert(connection);
    ValueHolder rv(xsink);
    Env env;
    try {
        QoreJdbcStatement res(xsink, this);

        bool result_set = res.exec(env, qstr, args, xsink);
        if (*xsink) {
            return QoreValue();
        }

        if (result_set) {
            return res.getOutputHash(env, xsink, false);
        }

        return res.rowsAffected(env);
    } catch (jni::Exception& e) {
        e.convert(xsink);
    }
    return *xsink ? QoreValue() : rv.release();
}
}
