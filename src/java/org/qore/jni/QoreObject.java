package org.qore.jni;

public class QoreObject {
    private long obj;

    public QoreObject(long obj) {
        this.obj = obj;
    }

    public long get() {
        return obj;
    }

    //! returns the class name for the object
    public String getClassName() {
        return getClassName0(obj);
    }

    //! calls the given method with the given arguments
    public Object callMethod(String name) {
        return callMethod0(obj, name);
    }

    //! calls the given method with the given arguments and returns the result
    public Object callMethod(String name, Object... args) {
        return callMethod0(obj, name, args);
    }

    //! returns the value of the given member
    public Object getMemberValue(String name) {
        return getMemberValue0(obj, name);
    }

    //! releases the Qore object without destroying it
    public void release() {
        long x = obj;
        obj = 0;
        release0(x);
    }

    //! runs the destructor
    public void destroy() {
        if (obj != 0) {
            long x = obj;
            obj = 0;
            destroy0(x);
        }
    }

    @Override
    protected void finalize() throws Throwable {
        if (obj != 0) {
            finalize0(obj);
            obj = 0;
        }
    }

    private native String getClassName0(long obj);
    private native Object callMethod0(long obj, String name);
    private native Object callMethod0(long obj, String name, Object... args);
    private native Object getMemberValue0(long obj, String name);
    private native void release0(long ptr);
    private native void destroy0(long ptr);
    private native void finalize0(long ptr);
}
