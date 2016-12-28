package org.qore.jni.test;

public class Fields {

    static Fields instance = new Fields();

    protected int pi = 1;
    private boolean z;
    byte b;
    char c;
    short s;
    int i;
    long j;
    float f;
    double d;

    byte[] ba = new byte[3];
    Object o;
    Object[] oa = new Object[3];
    Integer[] ia = new Integer[3];
}
