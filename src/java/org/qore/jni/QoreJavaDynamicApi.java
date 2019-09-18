package org.qore.jni;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.reflect.Field;

//! This class provides methods that allow Java to interface with Qore code
/**
 */
public class QoreJavaDynamicApi {
    //! invokes the given method on the given object and returns the return value
    public static Object invokeMethod(Method m, Object obj, Object... args) throws Throwable {
        try {
            Class<?> c = m.getDeclaringClass();
            m.trySetAccessible();
            return m.invoke(obj, args);
        } catch (InvocationTargetException e) {
            throw e.getCause();
        }
    }

    //! invokes the given method on the given object and returns the return value
    public static Object invokeMethodNonvirtual(Method m, Object obj, Object... args) throws Throwable {
        Class<?> c = m.getDeclaringClass();
        m.trySetAccessible();
        // works for all cases but generates a warning on the console if used with system classes
        return MethodHandles.privateLookupIn(c, MethodHandles.lookup()).unreflectSpecial(m, c).bindTo(obj).invokeWithArguments(args);
    }

    //! invokes the given method on the given object and returns the return value
    public static Object getField(Field f, Object obj) throws Throwable {
        f.setAccessible(true);
        return f.get(obj);
    }

    //! returns a lookup object for the program's context
    public static MethodHandles.Lookup lookup() {
        return MethodHandles.lookup();
    }
}
