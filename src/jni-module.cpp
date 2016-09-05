/*
  jni-module.cpp

  JNI integration to Qore

  Qore Programming Language

  Copyright (C) 2016 Qore Technologies, sro

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

  Note that the Qore library is released under a choice of three open-source
  licenses: MIT (as above), LGPL 2+, or GPL 2+; see README-LICENSE for more
  information.
*/
#include <qore/Qore.h>
#include "Jvm.h"

static QoreNamespace JniNamespace("Jni");

static QoreStringNode *jni_module_init();
static void jni_module_ns_init(QoreNamespace *rns, QoreNamespace *qns);
static void jni_module_delete();

DLLLOCAL void init_jni_functions(QoreNamespace& ns);
DLLLOCAL QoreClass* initClassClass(QoreNamespace& ns);
DLLLOCAL QoreClass* initMethodClass(QoreNamespace& ns);

DLLEXPORT char qore_module_name[] = "jni";
DLLEXPORT char qore_module_version[] = PACKAGE_VERSION;
DLLEXPORT char qore_module_description[] = "JNI module";
DLLEXPORT char qore_module_author[] = "Qore Technologies, sro";
DLLEXPORT char qore_module_url[] = "http://qore.org";
DLLEXPORT int qore_module_api_major = QORE_MODULE_API_MAJOR;
DLLEXPORT int qore_module_api_minor = QORE_MODULE_API_MINOR;
DLLEXPORT qore_module_init_t qore_module_init = jni_module_init;
DLLEXPORT qore_module_ns_init_t qore_module_ns_init = jni_module_ns_init;
DLLEXPORT qore_module_delete_t qore_module_delete = jni_module_delete;
#ifdef _QORE_HAS_QL_MIT
DLLEXPORT qore_license_t qore_module_license = QL_MIT;
#else
DLLEXPORT qore_license_t qore_module_license = QL_LGPL;
#endif
DLLEXPORT char qore_module_license_str[] = "MIT";

static void jni_thread_cleanup(void *) {
   jni::Jvm::threadCleanup();
}

static QoreStringNode *jni_module_init() {
   if (!jni::Jvm::createVM()) {
      return new QoreStringNode("unable to create Java VM");
   }
   tclist.push(jni_thread_cleanup, nullptr);

   JniNamespace.addSystemClass(initMethodClass(JniNamespace));
   JniNamespace.addSystemClass(initClassClass(JniNamespace));
   init_jni_functions(JniNamespace);

   return nullptr;
}

static void jni_module_ns_init(QoreNamespace *rns, QoreNamespace *qns) {
   qns->addInitialNamespace(JniNamespace.copy());
}

static void jni_module_delete() {
   tclist.pop(false);
   jni::Jvm::destroyVM();
}
