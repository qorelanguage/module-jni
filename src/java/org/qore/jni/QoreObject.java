package org.qore.jni;

public class QoreObject {
    private long obj;

    public QoreObject(long obj) {
        this.obj = obj;
    }

    public long get() {
        long x = obj;
        obj = 0;
        return x;
    }

    public String getClassName() {
        return getClassName0(obj);
    }

    public Object callMethod(String name) {
        return callMethod0(obj, name);
        //return null;
    }

    public Object callMethod(String name, Object... args) {
        return callMethod0(obj, name, args);
        //return null;
    }

    public Object getMemberValue(String name) {
        return getMemberValue0(obj, name);
        //return null;
    }

    @Override
    protected void finalize() throws Throwable {
        finalize0(obj);
        obj = 0;
    }

    private native String getClassName0(long obj);
    private native Object callMethod0(long obj, String name);
    private native Object callMethod0(long obj, String name, Object... args);
    private native Object getMemberValue0(long obj, String name);
    private native void finalize0(long ptr);
}
