// Copyright (C) 2026 Qore Technologies, s.r.o.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

package org.qore.jni.test;

import java.util.AbstractList;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.TreeMap;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Methods and fields whose types are Java lists and maps, for the declared %Qore types of Java collections.
 *
 * A Java list is converted to a %Qore list; a Java map is converted to a %Qore hash when all its keys are strings, so
 * only a map whose key type is String is always a hash.
 */
public class CollectionTypes {
    /** A list class of its own, with a constructor without arguments */
    public static class StringList extends ArrayList<String> {
        public StringList() {
        }
    }

    /** A list class without a constructor without arguments, which a %Qore list cannot be converted to */
    public static class FixedList extends AbstractList<String> {
        private final String[] values;

        public FixedList(String... values) {
            this.values = values;
        }

        @Override
        public String get(int index) {
            return values[index];
        }

        @Override
        public int size() {
            return values.length;
        }
    }

    /** A map class whose key type is fixed to String by its superclass */
    public static class StringMap extends HashMap<String, Object> {
        public StringMap() {
        }
    }

    /** A map class whose key type is a type parameter */
    public static class KeyedMap<K> extends LinkedHashMap<K, String> {
        public KeyedMap() {
        }
    }

    // lists
    public static ArrayList<String> arrayList() {
        return new ArrayList<String>(Arrays.asList("a", "b"));
    }

    public static LinkedList<Integer> linkedList() {
        return new LinkedList<Integer>(Arrays.asList(1, 2, 3));
    }

    public static List<String> list() {
        return Arrays.asList("x");
    }

    public static StringList stringList() {
        StringList rv = new StringList();
        rv.add("s");
        return rv;
    }

    public static FixedList fixedList() {
        return new FixedList("f1", "f2");
    }

    public static Collection<String> collection() {
        return new ArrayList<String>(Arrays.asList("c"));
    }

    // maps whose key type is String
    public static TreeMap<String, Integer> stringTreeMap() {
        TreeMap<String, Integer> rv = new TreeMap<String, Integer>();
        rv.put("one", 1);
        rv.put("two", 2);
        return rv;
    }

    public static ConcurrentHashMap<String, Object> concurrentMap() {
        ConcurrentHashMap<String, Object> rv = new ConcurrentHashMap<String, Object>();
        rv.put("k", "v");
        return rv;
    }

    public static Map<String, ?> wildcardMap() {
        return stringTreeMap();
    }

    public static Map<? extends String, Object> boundedKeyMap() {
        return new HashMap<String, Object>(stringTreeMap());
    }

    public static StringMap stringMap() {
        StringMap rv = new StringMap();
        rv.put("sm", 1);
        return rv;
    }

    public static KeyedMap<String> keyedStringMap() {
        KeyedMap<String> rv = new KeyedMap<String>();
        rv.put("ks", "v");
        return rv;
    }

    // maps whose key type is not String: a hash or a Java object, as the keys are
    public static TreeMap<Integer, String> integerTreeMap() {
        TreeMap<Integer, String> rv = new TreeMap<Integer, String>();
        rv.put(1, "one");
        return rv;
    }

    public static TreeMap<Integer, String> emptyIntegerTreeMap() {
        return new TreeMap<Integer, String>();
    }

    public static Properties properties() {
        Properties rv = new Properties();
        rv.setProperty("p", "v");
        return rv;
    }

    @SuppressWarnings("rawtypes")
    public static HashMap rawHashMap() {
        HashMap<String, String> rv = new HashMap<String, String>();
        rv.put("raw", "v");
        return rv;
    }

    public static Map<Object, Object> objectKeyMap() {
        Map<Object, Object> rv = new HashMap<Object, Object>();
        rv.put(1L, "one");
        return rv;
    }

    public static <K> Map<K, String> genericKeyMap(K key) {
        Map<K, String> rv = new HashMap<K, String>();
        rv.put(key, "v");
        return rv;
    }

    public static KeyedMap<Integer> keyedIntegerMap() {
        KeyedMap<Integer> rv = new KeyedMap<Integer>();
        rv.put(7, "seven");
        return rv;
    }

    // fields
    public static ArrayList<String> staticList = arrayList();
    public static TreeMap<String, Integer> staticStringMap = stringTreeMap();
    public static TreeMap<Integer, String> staticIntegerMap = integerTreeMap();

    public ArrayList<String> list = arrayList();
    public TreeMap<String, Integer> stringKeyed = stringTreeMap();
    public TreeMap<Integer, String> integerKeyed = integerTreeMap();

    // parameters: the class of the Java object that a %Qore value is converted to
    public static String describeArrayList(ArrayList<String> value) {
        return describe(value);
    }

    public static String describeLinkedList(LinkedList<Object> value) {
        return describe(value);
    }

    public static String describeStringList(StringList value) {
        return describe(value);
    }

    public static String describeFixedList(FixedList value) {
        return describe(value);
    }

    public static String describeTreeMap(TreeMap<String, Object> value) {
        return describe(value);
    }

    public static String describeProperties(Properties value) {
        return describe(value);
    }

    public static String describeConcurrentMap(ConcurrentHashMap<String, Object> value) {
        return describe(value);
    }

    public static String describeStringMap(StringMap value) {
        return describe(value);
    }

    // overloads with parameter types that a %Qore list or hash is converted to
    public static String overloadA(List<Object> value) {
        return "List";
    }

    public static String overloadA(ArrayList<Object> value) {
        return "ArrayList";
    }

    public static String overloadB(LinkedList<Object> value) {
        return "LinkedList";
    }

    public static String overloadB(ArrayList<Object> value) {
        return "ArrayList";
    }

    public static String overloadC(ArrayList<Object> value) {
        return "ArrayList";
    }

    public static String overloadC(Object[] value) {
        return "Object[]";
    }

    public static String overloadE(ArrayList<Object> value) {
        return "ArrayList";
    }

    public static String overloadE(CollectionTypes[] value) {
        return "CollectionTypes[]";
    }

    public static String overloadD(Map<String, Object> value) {
        return "Map";
    }

    public static String overloadD(TreeMap<String, Object> value) {
        return "TreeMap";
    }

    /** Returns the class of a value and its contents */
    public static String describe(Object value) {
        if (value == null) {
            return "null";
        }
        if (value instanceof Map) {
            return value.getClass().getSimpleName() + new TreeMap<Object, Object>((Map<?, ?>)value);
        }
        return value.getClass().getSimpleName() + value;
    }

    /** An interface whose methods a %Qore class implements */
    public interface Source {
        ArrayList<String> getList();

        TreeMap<String, Integer> getStringMap();

        TreeMap<Integer, String> getIntegerMap();

        int getCount();
    }

    /** Calls the methods of a Source through Java */
    public static String describeSource(Source src) {
        return describe(src.getList()) + "|" + describe(src.getStringMap()) + "|" + describe(src.getIntegerMap())
            + "|" + src.getCount();
    }
}
