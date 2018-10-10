package org.qore.jni;

import org.qore.jni.QoreURLClassLoader;

//! static API for Qore calls
public class QoreJavaApi {
    //! calls the given Qore function with the given arguments and returns the result
    public static Object callFunction(String name, Object... args) throws Throwable {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    //! calls the given Qore function with the given arguments and returns the result
    public static Object callFunctionArgs(String name, Object[] args) throws Throwable {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    private native static Object callFunction0(long pgm_ptr, String name, Object... args);
}
