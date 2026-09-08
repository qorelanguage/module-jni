// Copyright (C) 2026 Qore Technologies, s.r.o.
package org.qore.jni.test;

import java.lang.reflect.Array;
import java.util.Map;

/** Observes list conversions on the Java side, before conversion back to Qore. */
public final class ListConversions {
    private ListConversions() {
    }

    public static String describe(Object value) {
        if (value == null) {
            return "null";
        }
        return value.getClass().getName() + ":" + Array.getLength(value);
    }

    public static String describeEntry(Map<String, Object> value, String key) {
        return value.containsKey(key) ? describe(value.get(key)) : "absent";
    }

    public static Object roundTrip(Object value) {
        return value;
    }

    public static String describeObjects(Object[] value) {
        return describe(value);
    }

    public static String describeStrings(String[] value) {
        return describe(value);
    }

    public static String describeLongs(long[] value) {
        return describe(value);
    }

    public static String describeVarargs(String prefix, String... values) {
        return prefix + describe(values);
    }
}
