/* Copyright (C) 2026 Qore Technologies, s.r.o.
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
package org.qore.jni.test;

import org.qore.jni.QoreJavaApi;
import org.qore.jni.QoreObject;
import org.qore.jni.QoreURLClassLoader;

/** Regression for Qore cleanup when the JVM is the primary runtime.
 *
 * <p>A JVM-primary process never runs the Qore module-manager teardown by itself, so unless the
 * {@link QoreURLClassLoader} shutdown hook runs {@code qore_cleanup()}, libqore's static
 * destructors are reached with {@code QTF_EXTERNAL_LIFECYCLE} threads still running and the
 * process aborts at exit with "external native threads still active while stopping the cleanup
 * worker" - after {@code main()} has already completed successfully.
 *
 * <p>Both kinds of external-lifecycle worker are exercised below; the test asserts nothing itself,
 * its verdict is the process exit status.
 */
public class JvmPrimaryReaperExit {
    public static void main(String[] args) throws Throwable {
        QoreURLClassLoader loader = (QoreURLClassLoader)ClassLoader.getSystemClassLoader();
        loader.setContext();

        // Starting and stopping a ThreadPool leaves libqore's native completion
        // reaper idle on its condition variable until Qore runtime cleanup.
        QoreObject pool = QoreJavaApi.newObjectSave("Qore::Thread::ThreadPool", 1, 0, 0, 0);
        pool.callMethod("stopWait");

        // Socket I/O starts the process-wide async I/O controller, whose workers are
        // external-lifecycle threads that only stop in qore_cleanup().  A loopback listener
        // accepting a connection from this same process keeps the test hermetic.
        QoreObject listener = QoreJavaApi.newObjectSave("Qore::Socket");
        listener.callMethod("bindINET", "127.0.0.1", "0", true);
        listener.callMethod("listen");
        Object port = listener.callMethod("getPort");

        QoreObject client = QoreJavaApi.newObjectSave("Qore::Socket");
        client.callMethod("connectINET", "127.0.0.1", port.toString(), 5000);
    }
}
