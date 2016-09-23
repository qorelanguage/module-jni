package org.qore.jni;

public class QoreExceptionWrapper extends RuntimeException {

    private long xsink;

    QoreExceptionWrapper(long xsink) {
        this.xsink = xsink;
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

}
