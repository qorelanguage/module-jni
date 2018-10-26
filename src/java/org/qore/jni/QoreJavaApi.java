package org.qore.jni;

import org.qore.jni.QoreURLClassLoader;

//! This class provides methods that allow Java to interface with Qore code
/**
 */
public class QoreJavaApi {
    //! calls a Qore function and returns the result as a Java Object
    public static Object callFunction(String name, Object... args) {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    //! calls a Qore function and returns the result as a Java Object, takes a variable number of arguments
    public static Object callFunctionArgs(String name, Object[] args) {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    private native static Object callFunction0(long pgm_ptr, String name, Object... args);
}
