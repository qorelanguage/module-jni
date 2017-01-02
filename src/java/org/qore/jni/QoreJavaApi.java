package org.qore.jni;

import org.qore.jni.QoreURLClassLoader;

public class QoreJavaApi {
    public static Object callFunction(String name, Object... args) {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    public static Object callFunctionArgs(String name, Object[] args) {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    private native static Object callFunction0(long pgm_ptr, String name, Object... args);
}
