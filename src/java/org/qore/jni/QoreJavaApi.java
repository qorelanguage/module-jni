package org.qore.jni;

import org.qore.jni.QoreURLClassLoader;

import java.util.Arrays;

//! This class provides methods that allow Java to interface with Qore code
/**
 */
public class QoreJavaApi {
    //! Initialize the \c qore library and the \c jni module from a native Java thread
    /** This method allows \c Qore functionality or Java APIs backed by \c %Qore APIs to be used from native Java
        threads in an existing JVM process not started by \c %Qore itself.

        This requires the platform-dependent \c qore library to be found in a directory set in the
        \c java.library.path or by setting the \c QORE_LIBRARY environment variable to the absolute path of the
        \c qore library.

        If neither of these are set, then calling this method will result in a \c java.lang.UnsatisfiedLinkError
        exception being raised.

        This method is also safe to call when the Qore library and the jni module have been initialized already.
     */
    public static void initQore() throws Throwable {
        //System.out.println("DBG: QoreJavaApi::initQore()");
        try {
            QoreURLClassLoader.getProgramPtr();
        } catch (NullPointerException e0) {
            // initialize jni module
            long ptr;
            try {
                // try to get a Qore context for a Java thread not associated with a Qore Program object
                // but within a process initialized by the Qore library and the jni module
                ptr = initQore0();
            } catch (UnsatisfiedLinkError e1) {
                try {
                    // load the qore library from \c java.library.path
                    System.loadLibrary("qore");
                    // now we can run our native methods
                    ptr = initQore0();
                } catch (UnsatisfiedLinkError e2) {
                    // no qore library; no jni module
                    // check for environment variable to Qore lib
                    String qore_lib_loc = System.getenv("QORE_LIBRARY");
                    if (qore_lib_loc != null) {
                        // load the qore library dynamically, which will result in the jni module being loaded automatically
                        // do to a JNI_OnLoad() function in libqore that initializes the Qore library and loads the jni
                        // module
                        System.load(qore_lib_loc);
                        // now we can run our native methods
                        ptr = initQore0();
                    } else {
                        throw e2;
                    }
                }
            }
            QoreURLClassLoader.setProgramPtr(ptr);
        }
    }

    //! Calls the given Qore function with the given arguments and returns the result
    public static Object callFunction(String name, Object... args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(QoreURLClassLoader.getProgramPtr(), name, args);
    }

    //! Calls a Qore function and returns the result as a Java Object, takes a variable number of arguments
    public static Object callFunctionArgs(String name, Object[] args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunction0(QoreURLClassLoader.getProgramPtr(), name, args);
    }

    //! Calls the given Qore function with the given arguments and returns the result; if an object is returned, then a strong reference to the object is stored in thread-local data
    /**
     * This method can be used to save objects in thread-local data that would otherwise go out of scope.
     * The top-level hash key is determined by the value of the \c "_jni_save" thread-local key, if set, if
     * not then \c "_jni_save" is used instead as a literal key
     * @param name the name of the function to call
     * @param args argument to the function call
     * @return the result of the call
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static Object callFunctionSave(String name, Object... args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunctionSave0(QoreURLClassLoader.getProgramPtr(), name, args);
    }

    //! Calls the given Qore function with the given arguments and returns the result; if an object is returned, then a strong reference to the object is stored in thread-local data, takes a variable number of arguments
    /**
     * This method can be used to save objects in thread-local data that would otherwise go out of scope
     * The top-level hash key is determined by the value of the \c "_jni_save" thread-local key, if set, if
     * not then \c "_jni_save" is used instead as a literal key
     * @param name the name of the function to call
     * @param args argument to the function call
     * @return the result of the call
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static Object callFunctionSaveArgs(String name, Object[] args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callFunctionSave0(QoreURLClassLoader.getProgramPtr(), name, args);
    }

    //! Calls the given Qore static method with the given arguments and returns the result
    public static Object callStaticMethod(String class_name, String method_name, Object... args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callStaticMethod0(QoreURLClassLoader.getProgramPtr(), class_name, method_name, args);
    }

    //! Calls a Qore static method and returns the result as a Java Object, takes a variable number of arguments
    public static Object callStaticMethodArgs(String class_name, String method_name, Object[] args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callStaticMethod0(QoreURLClassLoader.getProgramPtr(), class_name, method_name, args);
    }

    //! Calls the given Qore static method with the given arguments and returns the result; if an object is returned, then a strong reference to the object is stored in thread-local data
    /**
     * This method can be used to save objects in thread-local data that would otherwise go out of scope.
     * The top-level hash key is determined by the value of the \c "_jni_save" thread-local key, if set, if
     * not then \c "_jni_save" is used instead as a literal key
     * @param class_name the name of the class where the static method is defined; can have a namespace-justified path
     * (ex: \c "Namespace::ClassName")
     * @param method_name the static method name to call
     * @param args argument to the static method call
     * @return the result of the call
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static Object callStaticMethodSave(String class_name, String method_name, Object... args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callStaticMethodSave0(QoreURLClassLoader.getProgramPtr(), class_name, method_name, args);
    }

    //! Calls the given Qore static method with the given arguments and returns the result; if an object is returned, then a strong reference to the object is stored in thread-local data, takes a variable number of arguments
    /**
     * This method can be used to save objects in thread-local data that would otherwise go out of scope
     * The top-level hash key is determined by the value of the \c "_jni_save" thread-local key, if set, if
     * not then \c "_jni_save" is used instead as a literal key
     * @param class_name the name of the class where the static method is defined; can have a namespace-justified path
     * (ex: \c "Namespace::ClassName")
     * @param method_name the static method name to call
     * @param args argument to the static method call
     * @return the result of the call
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static Object callStaticMethodSaveArgs(String class_name, String method_name, Object[] args) throws Throwable {
        //QoreURLClassLoader cl = QoreURLClassLoader.getCurrent();
        //System.out.println("cl: " + (cl != null ? cl.toString() : "null"));
        return callStaticMethodSave0(QoreURLClassLoader.getProgramPtr(), class_name, method_name, args);
    }

    //! Creates a new QoreObject from the class name and constructor arguments; a strong reference to the object is stored in thread-local data
    /**
     * @param class_name the class name or namespace-justified path (ex: \c "Qore::SQL::SQLStatement") of the object to create
     * @param args optional arguments to the constructor
     * @return the object created
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static QoreObject newObjectSave(String class_name, Object... args) throws Throwable {
        return newObjectSave0(QoreURLClassLoader.getProgramPtr(), class_name, args);
    }

    //! Creates a new QoreObject from the class name and constructor arguments; a strong reference to the object is stored in thread-local data under the given key
    /**
     * @param class_name the class name or namespace-justified path (ex: \c "Qore::SQL::SQLStatement") of the object to create
     * @param args optional arguments to the constructor
     * @return the object created
     * @throws Throwable any Qore-language exception is rethrown here
     */
    public static QoreObject newObjectSaveArgs(String class_name, Object[] args) throws Throwable {
        return newObjectSave0(QoreURLClassLoader.getProgramPtr(), class_name, args);
    }

    //! Returns the current stack trace, not including the call to this method
    public static StackTraceElement[] getStackTrace() {
        StackTraceElement[] stack = new Exception().getStackTrace();
        return stack.length > 0 ? Arrays.copyOfRange(stack, 1, stack.length) : null;
    }

    private native static long initQore0();
    private native static Object callFunction0(long pgm_ptr, String name, Object... args);
    private native static Object callFunctionSave0(long pgm_ptr, String name, Object... args);
    private native static Object callStaticMethod0(long pgm_ptr, String class_name, String method_name, Object... args);
    private native static Object callStaticMethodSave0(long pgm_ptr, String class_name, String method_name, Object... args);
    private native static QoreObject newObjectSave0(long pgm_ptr, String class_name, Object...args);
}
