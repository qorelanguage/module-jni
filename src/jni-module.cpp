/*
  jni-module.cpp

  JNI integration to Qore

  Qore Programming Language

  Copyright (C) 2016 Qore Technologies, s.r.o.

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
#include "QoreJniClassMap.h"

#define QORE_JNI_MODULE_NAME "jni"

#define QORE_JVM_SIGNALS SIGTRAP, SIGUSR1

#ifdef QORE_JVM_SIGNALS
#include <signal.h>
#include <pthread.h>
static int qore_jvm_sigs[] = { QORE_JVM_SIGNALS };
#define NUM_JVM_SIGS (sizeof(qore_jvm_sigs) / sizeof(int))
#endif

static QoreStringNode* jni_module_init();
static void jni_module_ns_init(QoreNamespace* rns, QoreNamespace* qns);
static void jni_module_delete();
static void jni_module_parse_cmd(const QoreString& cmd, ExceptionSink* xsink);

DLLLOCAL void preinitClassClass();

DLLEXPORT char qore_module_name[] = QORE_JNI_MODULE_NAME;
DLLEXPORT char qore_module_version[] = PACKAGE_VERSION;
DLLEXPORT char qore_module_description[] = "JNI module";
DLLEXPORT char qore_module_author[] = "Qore Technologies, s.r.o.";
DLLEXPORT char qore_module_url[] = "http://qore.org";
DLLEXPORT int qore_module_api_major = QORE_MODULE_API_MAJOR;
DLLEXPORT int qore_module_api_minor = QORE_MODULE_API_MINOR;
DLLEXPORT qore_module_init_t qore_module_init = jni_module_init;
DLLEXPORT qore_module_ns_init_t qore_module_ns_init = jni_module_ns_init;
DLLEXPORT qore_module_delete_t qore_module_delete = jni_module_delete;
DLLEXPORT qore_module_parse_cmd_t qore_module_parse_cmd = jni_module_parse_cmd;

DLLEXPORT qore_license_t qore_module_license = QL_MIT;
DLLEXPORT char qore_module_license_str[] = "MIT";

QoreJniClassMap qjcm;

static void jni_thread_cleanup(void*) {
   jni::Jvm::threadCleanup();
}

static QoreStringNode* jni_module_init() {
#ifdef QORE_JVM_SIGNALS
   {
      sigset_t mask;
      // setup signal mask
      sigemptyset(&mask);
      for (unsigned i = 0; i < NUM_JVM_SIGS; ++i) {
	 int sig = qore_jvm_sigs[i];
	 //printd(0, "jni_module_init() unblocking signal %d\n", sig);
	 sigaddset(&mask, sig);
	 // reassign signals needed by the JVM
	 QoreStringNode *err = qore_reassign_signal(sig, QORE_JNI_MODULE_NAME);
	 if (err)
	    return err;
      }
      // unblock threads
      pthread_sigmask(SIG_UNBLOCK, &mask, 0);
   }
#endif

   if (!jni::Jvm::createVM())
      return new QoreStringNode("unable to create Java VM");

   tclist.push(jni_thread_cleanup, nullptr);

   preinitClassClass();

   qjcm.init();
   return nullptr;
}

static void jni_module_ns_init(QoreNamespace* rns, QoreNamespace* qns) {
   qns->addNamespace(qjcm.getRootNS().copy());
}

static void jni_module_delete() {
   tclist.pop(false);
   jni::Jvm::destroyVM();
}

static void jni_module_parse_cmd(const QoreString& cmd, ExceptionSink* xsink) {
   const char* p = strchr(cmd.getBuffer(), ' ');

   if (!p) {
      xsink->raiseException("JNI-PARSE-COMMAND-ERROR", "missing command name in parse command: '%s'", cmd.getBuffer());
      return;
   }

   QoreString str(&cmd, p - cmd.getBuffer());
   if (strcmp(str.getBuffer(), "import")) {
      xsink->raiseException("JNI-PARSE-COMMAND-ERROR", "unrecognized command '%s' in '%s' (valid command: 'import')", str.getBuffer(), cmd.getBuffer());
      return;
   }

   QoreString arg(cmd);

   arg.replace(0, p - cmd.getBuffer() + 1, (const char *)0);
   arg.trim();

   QoreProgram* pgm = getProgram();
   bool has_feature = pgm ? pgm->checkFeature(QORE_JNI_MODULE_NAME) : false;

   // process import statement

   //printd(5, "jni_module_parse_cmd() pgm: %p arg: %s c: %c\n", pgm, arg.getBuffer(), arg[-1]);

   // see if there is a wildcard at the end
   bool wc = false;
   if (arg[-1] == '*') {
      if (arg[-2] != '.' || arg.strlen() < 3) {
	 xsink->raiseException("JNI-IMPORT-ERROR", "invalid import argument: '%s'", arg.getBuffer());
	 return;
      }

      arg.terminate(arg.strlen() - 2);

      arg.replaceAll(".", "::");

      QoreNamespace* jns;

      // create jni namespace in root namespace if necessary
      if (!has_feature)
	 jns = &qjcm.getRootNS();
      else {
	 QoreNamespace* rns = pgm->getRootNS();
	 jns = rns->findCreateNamespacePath("Jni");
      }

      QoreNamespace* ns = jns->findCreateNamespacePath(arg.getBuffer());
      ns->setClassHandler(jni_class_handler);
      wc = true;
   }
   else {
      /*
      // unblock signals and attach to java thread if necessary
      qjtr.check_thread();

      {
	 // parsing can occur in parallel in different QoreProgram objects
	 // so we need to protect the load with a lock
	 AutoLocker al(qjcm.m);
	 qjcm.loadClass(qjcm.getRootNS(), 0, arg.getBuffer(), 0, xsink);
      }
      */
   }

   // now try to add to current program
   //printd(5, "jni_module_parse_cmd() pgm: %p arg: %s\n", pgm, arg.getBuffer());
   if (!pgm)
      return;

   QoreNamespace* ns = pgm->getRootNS();

   //printd(5, "jni_module_parse_cmd() feature %s = %s (default_jns: %p)\n", QORE_JNI_MODULE_NAME, pgm->checkFeature(QORE_JNI_MODULE_NAME) ? "true" : "false", &qjcm.getRootNS());

   if (!pgm->checkFeature(QORE_JNI_MODULE_NAME)) {
      assert(qjcm.getRootNS().findLocalNamespace("java"));
      QoreNamespace *jns = qjcm.getRootNS().copy();

      //printd(5, "jns: %p %s\n", jns, jns->getName());

      assert(jns->findLocalNamespace("java"));

      ns->addNamespace(jns);
      pgm->addFeature(QORE_JNI_MODULE_NAME);

      ns = ns->findLocalNamespace("Jni");
      assert(ns);

      ns = ns->findLocalNamespace("java");
      assert(ns);

      return;
   }

   if (!wc) {
      ns = ns->findLocalNamespace("Jni");
      assert(ns);
      //qjcm.loadClass(*ns, 0, arg.getBuffer(), 0, xsink);
   }
}

QoreClass* jni_class_handler(QoreNamespace* ns, const char* cname) {
   // get full class path
   QoreString cp(ns->getName());
   cp.concat('.');
   cp.concat(cname);

   const QoreNamespace* jns = ns;
   while (true) {
      //printd(5, "jni_class_handler() ns: %p (%s) jns: %p (%s) cname: %s\n", ns, ns->getName(), jns, jns->getName(), cname);
      jns = jns->getParent();
      assert(jns);
      if (!strcmp(jns->getName(), "Jni"))
	 break;
      cp.prepend(".");
      cp.prepend(jns->getName());
   }

   //printd(0, "jni_class_handler() ns: %p cname: %s cp: %s\n", ns, cname, cp.getBuffer());

   return 0;

   /*
   // unblock signals and attach to java thread if necessary
   qjtr.check_thread();

   // class loading can occur in parallel in different QoreProgram objects
   // so we need to protect the load with a lock
   AutoLocker al(qjcm.m);
   QoreClass *qc = qjcm.loadClass(*const_cast<QoreNamespace *>(jns), 0, cp.getBuffer(), 0);

   //printd(5, "jni_class_handler() cp: %s returning qc: %p\n", cp.getBuffer(), qc);
   return qc;
   */
}
