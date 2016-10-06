package org.qore.jni.test;

public class FloatConversions {

    static boolean isNaNFloat(float f) {
        return Float.isNaN(f);
    }

    static boolean isNInfFloat(float f) {
        return Float.isInfinite(f) && f < 0;
    }

    static boolean isPInfFloat(float f) {
        return Float.isInfinite(f) && f > 0;
    }

    static float getNaNFloat() {
        return Float.NaN;
    }

    static float getNInfFloat() {
        return Float.NEGATIVE_INFINITY;
    }

    static float getPInfFloat() {
        return Float.POSITIVE_INFINITY;
    }

    static boolean isNaNDouble(double d) {
        return Double.isNaN(d);
    }

    static boolean isNInfDouble(double d) {
        return Double.isInfinite(d) && d < 0;
    }

    static boolean isPInfDouble(double d) {
        return Double.isInfinite(d) && d > 0;
    }

    static double getNaNDouble() {
        return Double.NaN;
    }

    static double getNInfDouble() {
        return Double.NEGATIVE_INFINITY;
    }

    static double getPInfDouble() {
        return Double.POSITIVE_INFINITY;
    }
}

