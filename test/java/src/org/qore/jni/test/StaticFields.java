package org.qore.jni.test;

public class StaticFields {

    private static boolean z;
    static byte b;
    static char c;
    static short s;
    static int i;
    static long j;
    static float f;
    static double d;
    static Number n;
    static Long l;

    static byte[] ba = new byte[3];
    static Object o;
    static Object[] oa = new Object[3];
    static Integer[] ia = new Integer[3];

    static Integer wrap(int i) {
        return i;
    }

    static int unwrapN() {
        return n.intValue();
    }
}
