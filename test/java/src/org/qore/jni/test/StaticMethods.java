package org.qore.jni.test;

public class StaticMethods {
    private static int i;

    public static void set(int i) {
        StaticMethods.i = i;
    }

    public static int get() {
        return i;
    }

    public static boolean cmp(short s, long l) {
        return s > l;
    }

    public static char add(byte a, char b) {
        return (char) (a + b);
    }

    public static long get(boolean fail) {
        if (fail) {
            throw new RuntimeException("fail is true");
        }
        return Long.MAX_VALUE;
    }

    public static byte avg(byte b1, byte b2, byte b3) {
        return (byte) ((b1 + b2 + b3) / 3);
    }

    public static short max(long l1, long l2) {
        return l1 > l2 ? (short) l1 : (short) l2;
    }

    public static float avg(float f1, float f2) {
        return (f1 + f2) / 2;
    }

    public static double avg(double d1, double d2) {
        return (d1 + d2) / 2;
    }

    public static Integer wrap(int i) {
        return i;
    }

    public static int unwrap(Integer i) {
        return i == null ? -123 : i;
    }

    public static Long[] getLongArray() {
        return new Long[3];
    }

    public static void useArray(Object[] a) {
    }

    public static Object conversions(String name) throws Exception {
        if (name.equals("method")) {
            return Object.class.getMethod("toString", new Class[] {});
        }
        if (name.equals("static method")) {
            return Integer.class.getMethod("valueOf", new Class[] {int.class});
        }
        if (name.equals("constructor")) {
            return Integer.class.getConstructor(new Class[] {int.class});
        }
        if (name.equals("field")) {
            return Fields.class.getDeclaredField("i");
        }
        if (name.equals("static field")) {
            return StaticFields.class.getDeclaredField("i");
        }
        return null;
    }
}
