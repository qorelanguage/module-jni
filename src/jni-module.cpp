/* -*- indent-tabs-mode: nil -*- */
/*
    jni-module.cpp

    JNI integration to Qore

    Qore Programming Language

    Copyright (C) 2016 - 2017 Qore Technologies, s.r.o.

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

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <vector>

#include "defs.h"
#include "Jvm.h"
#include "QoreJniClassMap.h"
#include "Method.h"
#include "QoreToJava.h"

using namespace jni;

#define QORE_JNI_MODULE_NAME "jni"

#ifndef Q_WINDOWS
#include <signal.h>
#include <pthread.h>
#endif

sig_vec_t sig_vec = {
#ifndef Q_WINDOWS
    SIGTRAP, SIGUSR1, SIGSEGV, SIGBUS
#endif
};

static QoreStringNode* jni_module_init();
static void jni_module_ns_init(QoreNamespace* rns, QoreNamespace* qns);
static void jni_module_delete();
static void jni_module_parse_cmd(const QoreString& cmd, ExceptionSink* xsink);

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

// module cmd type
typedef void (qore_jni_module_cmd_t)(QoreString& arg);
static void qore_jni_mc_import(QoreString& arg);
static void qore_jni_mc_add_classpath(QoreString& arg);
static void qore_jni_mc_add_relative_classpath(QoreString& arg);

// module cmds
typedef std::map<std::string, std::function<qore_jni_module_cmd_t>> mcmap_t;
static mcmap_t mcmap = {
    {"import", qore_jni_mc_import},
    {"add-classpath", qore_jni_mc_add_classpath},
    {"add-relative-classpath", qore_jni_mc_add_relative_classpath},
};

static void jni_thread_cleanup(void*) {
    jni::Jvm::threadCleanup();
}

static QoreStringNode* jni_module_init() {
    printd(LogLevel, "jni_module_init()\n");

    QoreStringNode* err = jni::Jvm::createVM();
    if (err) {
        err->prepend("Could not create the Java Virtual Machine: ");
        return err;
    }

   //printd(5, "jni_module_init() initialized JVM\n");

#ifndef Q_WINDOWS
    {
        QoreStringNode *err = qore_reassign_signals(sig_vec, QORE_JNI_MODULE_NAME);
        if (err) {
            jni::Jvm::destroyVM();
            return err;
        }
        sigset_t mask;
        // setup signal mask
        sigemptyset(&mask);
        for (auto& sig : sig_vec) {
            //printd(LogLevel, "jni_module_init() unblocking signal %d\n", sig);
            sigaddset(&mask, sig);
        }
        // unblock threads
        pthread_sigmask(SIG_UNBLOCK, &mask, 0);
    }
#endif

    tclist.push(jni_thread_cleanup, nullptr);

    try {
        qjcm.init();
    }
    catch (jni::Exception& e) {
        // display exception info on the console as an unhandled exception
        {
            ExceptionSink xsink;
            e.convert(&xsink);
        }
        tclist.pop(false);
        qore_release_signals(sig_vec, QORE_JNI_MODULE_NAME);
        jni::Jvm::destroyVM();
        return new QoreStringNode("ERR");
    }

    return nullptr;
}

static void jni_module_ns_init(QoreNamespace* rns, QoreNamespace* qns) {
   QoreProgram* pgm = getProgram();
   assert(pgm->getRootNS() == rns);
   if (!pgm->getExternalData("jni")) {
      QoreNamespace* jnins = qjcm.getJniNs().copy();
      rns->addNamespace(jnins);
      pgm->setExternalData("jni", new JniExternalProgramData(jnins));
   }
}

static void jni_module_delete() {
   // clear all objects from stored classes before destroying the JVM (releases all global references)
   {
      ExceptionSink xsink;
      qjcm.destroy(xsink);
   }
   tclist.pop(false);
   jni::Jvm::destroyVM();
}

static void jni_module_parse_cmd(const QoreString& cmd, ExceptionSink* xsink) {
   printd(LogLevel, "jni_module_parse_cmd() cmd: '%s'\n", cmd.c_str());

   const char* p = strchr(cmd.getBuffer(), ' ');

   if (!p) {
      xsink->raiseException("JNI-PARSE-COMMAND-ERROR", "missing command name in parse command: '%s'", cmd.getBuffer());
      return;
   }

   QoreString str(&cmd, p - cmd.getBuffer());

   QoreString arg(cmd);

   arg.replace(0, p - cmd.getBuffer() + 1, (const char*)0);
   arg.trim();

   mcmap_t::const_iterator i = mcmap.find(str.c_str());
   if (i == mcmap.end()) {
      QoreStringNode* desc = new QoreStringNodeMaker("unrecognized command '%s' in '%s' (valid commands: ", str.c_str(), cmd.c_str());
      for (mcmap_t::const_iterator i = mcmap.begin(), e = mcmap.end(); i != e; ++i) {
         if (i != mcmap.begin())
            desc->concat(", ");
         desc->sprintf("'%s'", i->first.c_str());
      }
      desc->concat(')');
      xsink->raiseException("JNI-PARSE-COMMAND-ERROR", desc);
      return;
   }

   QoreProgram* pgm = getProgram();
   if (!pgm->getExternalData("jni")) {
      QoreNamespace* jnins = qjcm.getJniNs().copy();
      pgm->getRootNS()->addNamespace(jnins);
      pgm->setExternalData("jni", new JniExternalProgramData(jnins));
      pgm->addFeature(QORE_JNI_MODULE_NAME);
   }

   try {
      i->second(arg);
   }
   catch (jni::Exception& e) {
      e.convert(xsink);
   }
}

static void qore_jni_mc_import(QoreString& arg) {
   QoreProgram* pgm = getProgram();
   assert(pgm);
   assert(pgm->checkFeature(QORE_JNI_MODULE_NAME));

   // process import statement
   printd(LogLevel, "jni_module_parse_cmd() pgm: %p arg: %s c: %c\n", pgm, arg.getBuffer(), arg[-1]);

   // see if there is a wildcard at the end
   bool wc = false;
   if (arg[-1] == '*') {
      if (arg[-2] != '.' || arg.strlen() < 3) {
         throw QoreJniException("JNI-IMPORT-ERROR", "invalid import argument: '%s'", arg.getBuffer());
         return;
      }

      arg.terminate(arg.strlen() - 2);

      arg.replaceAll(".", "::");
      arg.concat("::x");

      QoreNamespace* jns;

      // create jni namespace in root namespace if necessary
      jns = pgm->getRootNS()->findLocalNamespace("Jni");

      QoreNamespace* ns = jns->findCreateNamespacePath(arg.c_str());
      printd(LogLevel, "jni_module_parse_cmd() nsp: '%s' ns: %p '%s'\n", arg.c_str(), ns, ns->getName());
      ns->setClassHandler(jni_class_handler);
      wc = true;
   }
   else {
      printd(LogLevel, "jni_module_parse_cmd() non wc lcc arg: '%s'\n", arg.c_str());
      qjcm.findCreateQoreClass(arg.c_str());
   }

   // now try to add to current program
   printd(LogLevel, "jni_module_parse_cmd() pgm: %p arg: '%s'\n", pgm, arg.c_str());

   QoreNamespace* ns = pgm->getRootNS();

   printd(LogLevel, "jni_module_parse_cmd() feature '%s': %s (default_jns: %p)\n", QORE_JNI_MODULE_NAME, pgm->checkFeature(QORE_JNI_MODULE_NAME) ? "true" : "false", &qjcm.getJniNs());

   if (!pgm->checkFeature(QORE_JNI_MODULE_NAME)) {
      QoreNamespace* jns = qjcm.getJniNs().copy();
      printd(LogLevel, "jns: %p '%s' ns: %p\n", jns, jns->getName(), ns);

      assert(jns->findLocalNamespace("java"));

      ns->addNamespace(jns);
      pgm->addFeature(QORE_JNI_MODULE_NAME);

      return;
   }

   if (!wc) {
      QoreString jpath(arg);
      jpath.replaceAll(".", "/");
      qjcm.findLoadClass(jpath.c_str());
   }
}

static void qore_jni_mc_add_classpath(QoreString& arg) {
   q_env_subst(arg);
   printd(LogLevel, "qore_jni_mc_add_classpath() arg: '%s'\n", arg.c_str());

   JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(getProgram()->getExternalData("jni"));
   jpc->addClasspath(arg.c_str());
}

static void qore_jni_mc_add_relative_classpath(QoreString& arg) {
   SimpleRefHolder<QoreStringNode> cwd_str;

   QoreProgram* pgm = getProgram();
   assert(pgm);
   cwd_str = pgm->getScriptDir();

   if (!cwd_str) {
      char* cwd = getcwd(0, 0);
      if (!cwd)
         throw QoreJniException("JNI-ADD-RELATIVE-CLASSPATH-ERROR", "cannot determine relative path; there is no information in the Program context and cannot get current working directory: %s", strerror(errno));
      ON_BLOCK_EXIT(free, cwd);
      cwd_str = new QoreStringNode(cwd);
   }

   cwd_str->concat(QORE_DIR_SEP);
   cwd_str->concat(arg.c_str());
   q_normalize_path(**cwd_str);

   cwd_str->concat('/');

   printd(LogLevel, "qore_jni_mc_add_relative_classpath() arg: '%s' cwd: '%s'\n", arg.c_str(), cwd_str->c_str());

   JniExternalProgramData* jpc = static_cast<JniExternalProgramData*>(getProgram()->getExternalData("jni"));
   jpc->addClasspath(cwd_str->c_str());
}

QoreClass* jni_class_handler(QoreNamespace* ns, const char* cname) {
   // get full class path
   QoreString cp(ns->getName());
   cp.concat('.');
   cp.concat(cname);

   const QoreNamespace* jns = ns;
   while (true) {
      printd(LogLevel, "jni_class_handler() ns: %p (%s) jns: %p (%s) cname: %s\n", ns, ns->getName(), jns, jns->getName(), cname);
      jns = jns->getParent();
      assert(jns);
      if (!strcmp(jns->getName(), "Jni"))
         break;
      cp.prepend(".");
      cp.prepend(jns->getName());
   }

   printd(LogLevel, "jni_class_handler() ns: %p cname: %s cp: %s\n", ns, cname, cp.getBuffer());

   try {
      QoreClass* qc = qjcm.findCreateQoreClass(cp.getBuffer());
      printd(LogLevel, "jni_class_handler() cp: %s returning qc: %p\n", cp.getBuffer(), qc);
      return qc;
   }
   catch (jni::JavaException& e) {
      // ignore class not found exceptions here
      e.ignoreOrRethrowNoClass();
   }
   catch (jni::Exception& e) {
      // display exception info on the console as an unhandled exception
      {
         ExceptionSink xsink;
         e.convert(&xsink);
      }
      assert(false);
   }
   return 0;
}
