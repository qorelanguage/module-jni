/*  JavaCollectionTypes.java Copyright 2026 Qore Technologies, s.r.o.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

package org.qore.jni;

import java.lang.reflect.Field;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.Method;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

/**
 * Determines from generic signatures whether a Java map is always converted to a %Qore hash
 *
 * The jni module converts a Java map to a %Qore hash when all its keys are strings, and leaves it a Java object
 * otherwise; a map is therefore always a hash only when its declared key type is String.  The key type is resolved
 * through the type arguments of the declared type and of its superclasses and interfaces, so that
 * <tt>Map&lt;String, V&gt;</tt>, <tt>TreeMap&lt;String, V&gt;</tt>, and a class declared as
 * <tt>StringMap extends HashMap&lt;String, Object&gt;</tt> have String keys, while a raw <tt>Map</tt>,
 * <tt>Map&lt;K, V&gt;</tt> with a type variable <tt>K</tt>, <tt>Properties</tt>
 * (<tt>Hashtable&lt;Object, Object&gt;</tt>), and <tt>TreeMap&lt;Integer, V&gt;</tt> do not.
 *
 * This class is used by the jni module when it creates the %Qore classes for Java classes; it is not an API.
 */
final class JavaCollectionTypes {
    private JavaCollectionTypes() {
    }

    /**
     * Returns the sum of the depths of the parameter classes of a method in the type hierarchy, where the depth of a class is 0 for a class
     * without a superclass or interfaces (ex: Object, an interface without superinterfaces), and 1 more than the
     * greatest depth of its superclass and interfaces otherwise; a subclass is always deeper than its superclass and
     * interfaces, so a method whose parameter classes are subclasses of those of another method has a greater sum
     *
     * @param executable a method or constructor, whose parameter classes are summed
     */
    static int getDepth(java.lang.reflect.Executable executable) {
        int rv = 0;
        for (Class<?> cls : executable.getParameterTypes()) {
            rv += getClassDepth(cls);
        }
        return rv;
    }

    /**
     * Returns the depth of a class in the type hierarchy; see getDepth()
     */
    private static int getClassDepth(Class<?> cls) {
        int rv = 0;
        Class<?> sup = cls.getSuperclass();
        if (sup != null) {
            rv = getClassDepth(sup) + 1;
        }
        for (Class<?> iface : cls.getInterfaces()) {
            rv = Math.max(rv, getClassDepth(iface) + 1);
        }
        return rv;
    }

    /**
     * Returns true if the declared type of a method's return value, of a field, or the given type is a map whose key
     * type is String
     *
     * @param typeOrMember a java.lang.reflect.Method (its generic return type), a java.lang.reflect.Field (its
     * generic type), or a java.lang.reflect.Type
     */
    static boolean isStringKeyMap(Object typeOrMember) {
        Type type;
        if (typeOrMember instanceof Method) {
            type = ((Method)typeOrMember).getGenericReturnType();
        } else if (typeOrMember instanceof Field) {
            type = ((Field)typeOrMember).getGenericType();
        } else if (typeOrMember instanceof Type) {
            type = (Type)typeOrMember;
        } else {
            return false;
        }
        return isStringType(getMapKeyType(type, new HashMap<TypeVariable<?>, Type>(),
            new HashSet<TypeVariable<?>>()));
    }

    /**
     * Returns true if a value declared with a class can be a Java list or map that the jni module converts to a %Qore
     * list or hash, without the class being a list or map class: an interface or class that the common list and map
     * classes implement or extend, such as Collection, Iterable, Queue, Serializable, or Object
     *
     * @param cls the declared class
     */
    static boolean mayBeListOrMap(Class<?> cls) {
        if (List.class.isAssignableFrom(cls) || Map.class.isAssignableFrom(cls)) {
            return false;
        }
        return cls.isAssignableFrom(ArrayList.class) || cls.isAssignableFrom(LinkedList.class)
            || cls.isAssignableFrom(HashMap.class) || cls.isAssignableFrom(TreeMap.class);
    }

    /**
     * Returns the type that the key type parameter of java.util.Map has in a type, or null if the type is not a map
     * or the key type cannot be determined
     *
     * @param type the type
     * @param bindings the types of the type variables that the type refers to
     * @param seen the type variables being resolved, against recursive bounds (ex: <tt>K extends Comparable&lt;K&gt;</tt>)
     */
    private static Type getMapKeyType(Type type, Map<TypeVariable<?>, Type> bindings, Set<TypeVariable<?>> seen) {
        if (type instanceof TypeVariable) {
            TypeVariable<?> var = (TypeVariable<?>)type;
            Type bound = bindings.get(var);
            if (bound != null) {
                return getMapKeyType(bound, bindings, seen);
            }
            // a type variable is a map if its first bound is one
            if (!seen.add(var)) {
                return null;
            }
            Type[] bounds = var.getBounds();
            return bounds.length == 0 ? null : getMapKeyType(bounds[0], bindings, seen);
        }
        if (type instanceof WildcardType) {
            Type[] upper = ((WildcardType)type).getUpperBounds();
            return upper.length == 0 ? null : getMapKeyType(upper[0], bindings, seen);
        }
        if (type instanceof GenericArrayType) {
            return null;
        }

        Class<?> raw;
        Map<TypeVariable<?>, Type> class_bindings = new HashMap<TypeVariable<?>, Type>();
        if (type instanceof ParameterizedType) {
            ParameterizedType ptype = (ParameterizedType)type;
            if (!(ptype.getRawType() instanceof Class)) {
                return null;
            }
            raw = (Class<?>)ptype.getRawType();
            TypeVariable<?>[] vars = raw.getTypeParameters();
            Type[] args = ptype.getActualTypeArguments();
            for (int i = 0; i < vars.length && i < args.length; ++i) {
                // an argument that is a type variable of the context takes its binding
                Type arg = args[i];
                if (arg instanceof TypeVariable && bindings.containsKey(arg)) {
                    arg = bindings.get(arg);
                }
                class_bindings.put(vars[i], arg);
            }
        } else if (type instanceof Class) {
            // a raw type: its type variables are not bound
            raw = (Class<?>)type;
        } else {
            return null;
        }

        if (!Map.class.isAssignableFrom(raw)) {
            return null;
        }
        if (raw == Map.class) {
            Type key = class_bindings.get(Map.class.getTypeParameters()[0]);
            return key;
        }

        // find the key type through the supertypes of the class
        for (Type iface : raw.getGenericInterfaces()) {
            if (isMap(iface)) {
                return getMapKeyType(iface, class_bindings, seen);
            }
        }
        Type sup = raw.getGenericSuperclass();
        if (sup != null && isMap(sup)) {
            return getMapKeyType(sup, class_bindings, seen);
        }
        return null;
    }

    /**
     * Returns true if the raw class of a type is a map
     */
    private static boolean isMap(Type type) {
        if (type instanceof Class) {
            return Map.class.isAssignableFrom((Class<?>)type);
        }
        if (type instanceof ParameterizedType) {
            Type raw = ((ParameterizedType)type).getRawType();
            return raw instanceof Class && Map.class.isAssignableFrom((Class<?>)raw);
        }
        return false;
    }

    /**
     * Returns true if a type is String: the class, or a type variable or wildcard bounded by it (String is final)
     */
    private static boolean isStringType(Type type) {
        if (type == String.class) {
            return true;
        }
        if (type instanceof WildcardType) {
            Type[] upper = ((WildcardType)type).getUpperBounds();
            return upper.length > 0 && upper[0] == String.class;
        }
        if (type instanceof TypeVariable) {
            Type[] bounds = ((TypeVariable<?>)type).getBounds();
            return bounds.length > 0 && bounds[0] == String.class;
        }
        return false;
    }
}
