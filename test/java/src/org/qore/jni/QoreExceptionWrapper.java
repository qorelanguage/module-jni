package org.qore.jni;

public class QoreExceptionWrapper extends RuntimeException {

    private long xsink;

    QoreExceptionWrapper(long xsink) {
        this.xsink = xsink;
    }

    @Override
    public String getMessage() {
        return getMessage0(xsink);
    }

    long get() {
        long x = xsink;
        xsink = 0;
        return x;
    }

    @Override
    protected void finalize() throws Throwable {
        finalize0(xsink);
        xsink = 0;
    }

    private native void finalize0(long ptr);
    private native String getMessage0(long ptr);
}
