/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
    QoreJniClassMap.h

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

#ifndef _QORE_JNI_QOREJNIPRIVATEDATA_H

#define _QORE_JNI_QOREJNIPRIVATEDATA_H

#include "GlobalReference.h"
#include "LocalReference.h"

namespace jni {

class QoreJniPrivateData : public AbstractPrivateData {
public:
   DLLLOCAL QoreJniPrivateData(jobject n_jobj) : jobj(GlobalReference<jobject>::fromLocal(n_jobj)) {
   }

   template <typename T>
   DLLLOCAL T cast() const {
      return jobj.cast<T>();
   }

   DLLLOCAL jobject getObject() const {
      return jobj;
   }

   DLLLOCAL LocalReference<jobject> makeLocal() const {
      return jobj.toLocal();
   }

protected:
   GlobalReference<jobject> jobj;

   DLLLOCAL QoreJniPrivateData() = default;
};
}
#endif
