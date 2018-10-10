package org.qore.jni.test;

import org.qore.jni.QoreJavaApi;

public class QoreCallback {
    static Object callFunctionTest() throws Throwable {
        return QoreJavaApi.callFunction("test_func");
    }
}
