/*
    QoreJavaDynamicApoi.java

    Qore Programming Language JNI Module

    Copyright (C) 2016 - 2022 Qore Technologies, s.r.o.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

package org.qore.jni;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;

//! This class provides methods that allow Java to interface with Qore code
/**
 */
public class QoreJavaDynamicApi {
    //! creates an instance of the given class
    public static Object newInstance(Constructor<?> c, Object... args) throws Throwable {
        try {
            c.trySetAccessible();
            return c.newInstance(args);
        } catch (InvocationTargetException e) {
            Throwable e0 = e;
            while (e0 instanceof InvocationTargetException) {
                e0 = e0.getCause();
            }
            throw e0;
        }
    }

    //! invokes the given method on the given object and returns the return value
    public static Object invokeMethod(Method m, Object obj, Object... args) throws Throwable {
        try {
            m.trySetAccessible();
            return m.invoke(obj, args);
        } catch (InvocationTargetException e) {
            Throwable e0 = e;
            while (e0 instanceof InvocationTargetException) {
                e0 = e0.getCause();
            }
            throw e0;
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
