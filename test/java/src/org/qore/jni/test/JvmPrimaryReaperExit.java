/* Copyright (C) 2026 Qore Technologies, s.r.o.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
package org.qore.jni.test;

import org.qore.jni.QoreJavaApi;
import org.qore.jni.QoreObject;
import org.qore.jni.QoreURLClassLoader;

/** Regression for Qore cleanup when the JVM is the primary runtime. */
public class JvmPrimaryReaperExit {
    public static void main(String[] args) throws Throwable {
        QoreURLClassLoader loader = (QoreURLClassLoader)ClassLoader.getSystemClassLoader();
        loader.setContext();

        // Starting and stopping a ThreadPool leaves libqore's native completion
        // reaper idle on its condition variable until Qore runtime cleanup.
        QoreObject pool = QoreJavaApi.newObjectSave("Qore::Thread::ThreadPool", 1, 0, 0, 0);
        pool.callMethod("stopWait");
    }
}
