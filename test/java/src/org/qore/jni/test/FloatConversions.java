package org.qore.jni.test;

public class FloatConversions {
    public static boolean isNaNFloat(float f) {
        return Float.isNaN(f);
    }

    public static boolean isNInfFloat(float f) {
        return Float.isInfinite(f) && f < 0;
    }

    public static boolean isPInfFloat(float f) {
        return Float.isInfinite(f) && f > 0;
    }

    public static float getNaNFloat() {
        return Float.NaN;
    }

    public static float getNInfFloat() {
        return Float.NEGATIVE_INFINITY;
    }

    public static float getPInfFloat() {
        return Float.POSITIVE_INFINITY;
    }

    public static boolean isNaNDouble(double d) {
        return Double.isNaN(d);
    }

    public static boolean isNInfDouble(double d) {
        return Double.isInfinite(d) && d < 0;
    }

    public static boolean isPInfDouble(double d) {
        return Double.isInfinite(d) && d > 0;
    }

    public static double getNaNDouble() {
        return Double.NaN;
    }

    public static double getNInfDouble() {
        return Double.NEGATIVE_INFINITY;
    }

    public static double getPInfDouble() {
        return Double.POSITIVE_INFINITY;
    }
}

