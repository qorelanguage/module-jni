package org.qore.jni;

import org.qore.jni.QoreURLClassLoader;

//! This class provides methods that allow Java to interface with Qore code
/**
 */
public class QoreJavaApi {
    //! Calls the given Qore function with the given arguments and returns the result
    public static Object callFunction(String name, Object... args) throws Throwable {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    //! Calls a Qore function and returns the result as a Java Object, takes a variable number of arguments
    public static Object callFunctionArgs(String name, Object[] args) throws Throwable {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(cl.getProgramPtr(), name, args);
    }

    //! Calls the given Qore function with the given arguments and returns the result; if an object is returned, then a strong reference to the object is stored in thread-local data under the given domain and key
    /**
     * This method can be used to save objects in thread-local data that would otherwise go out of scope.
     * The top-level hash key is determined by the value of the \c "_jni_save" thread-local key, if set, if
     * not then \c "_jni_save" is used instead as a literal key
     * @param key the second-level key for the thread-local data where the strong reference to the object will be stored
     * @param name the name of the function to call
     * @param args argument to the function call
     * @return the result of the call
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static Object callFunctionSave(String key, String name, Object... args) throws Throwable {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunctionSave0(cl.getProgramPtr(), key, name, args);
    }

    //! Calls the given Qore function with the given arguments and returns the result; if an object is returned, then a strong reference to the object is stored in thread-local data under the given domain and key, takes a variable number of arguments
    /**
     * This method can be used to save objects in thread-local data that would otherwise go out of scope
     * The top-level hash key is determined by the value of the \c "_jni_save" thread-local key, if set, if
     * not then \c "_jni_save" is used instead as a literal key
     * @param key the second-level key for the thread-local data where the strong reference to the object will be stored
     * @param name the name of the function to call
     * @param args argument to the function call
     * @return the result of the call
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static Object callFunctionSaveArgs(String key, String name, Object[] args) throws Throwable {
        QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunctionSave0(cl.getProgramPtr(), key, name, args);
    }

    private native static Object callFunction0(long pgm_ptr, String name, Object... args);
    private native static Object callFunctionSave0(long pgm_ptr, String key, String name, Object... args);
}
