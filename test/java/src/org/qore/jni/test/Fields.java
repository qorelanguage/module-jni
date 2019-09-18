package org.qore.jni.test;

public class Fields {
    public static Fields instance = new Fields();

    protected int pi = 1;
    private boolean z;

    public byte b;
    public char c;
    public short s;
    public int i;
    public long j;
    public float f;
    public double d;

    public byte[] ba = new byte[3];
    public Object o;
    public Object[] oa = new Object[3];
    public Integer[] ia = new Integer[3];
}
