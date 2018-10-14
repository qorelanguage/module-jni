package org.qore.jni;

//! wrapper class for a Qore object; this class holds a weak reference to the Qore object
/** @note API usage errors such as with releasing / deleting the object and then calling methods
    on the object will cause a crash
 */
public class QoreObject {
    //! a pointer to the Qore object
    private long obj;

    //! creates the wrapper object with a pointer to an object; this JAva object holds a weak reference to the Qore object passed here
    public QoreObject(long obj) {
        this.obj = obj;
    }

    //! returns the pointer to the object
    public long get() {
        return obj;
    }

    //! returns the class name for the object
    public String className() {
        return className0(obj);
    }

    //! returns true if the object is an instance of the given class
    public boolean instanceOf(String class_name) {
        return instanceOf0(obj, class_name);
    }

    //! calls the given method with the given arguments
    public Object callMethod(String name) throws Throwable {
        return callMethod0(obj, name);
    }

    //! calls the given method with the given arguments and returns the result
    public Object callMethod(String name, Object... args) throws Throwable {
        return callMethod0(obj, name, args);
    }

    //! returns the value of the given member
    public Object getMemberValue(String name) throws Throwable {
        return getMemberValue0(obj, name);
    }

    //! releases the Qore object without destroying it
    /** @note if the object is returned to Qore, do not release it; allow the weak reference
        to be released when finalized
     */
    public void release() {
        long x = releasePointer();
        if (x != 0) {
            release0(x);
        }
    }

    //! runs the destructor
    public void destroy() {
        long x = releasePointer();
        if (x != 0) {
            destroy0(x);
        }
    }

    //! releases the weak reference
    @Override
    protected void finalize() throws Throwable {
        if (obj != 0) {
            finalize0(obj);
            obj = 0;
        }
    }

    //! clears the internal pointer and returns the pointer value as a long
    private long releasePointer() {
        long x = obj;
        obj = 0;
        return x;
    }

    private native String className0(long ptr);
    private native boolean instanceOf0(long ptr, String class_name);
    private native Object callMethod0(long ptr, String name);
    private native Object callMethod0(long ptr, String name, Object... args);
    private native Object getMemberValue0(long ptr, String name);
    private native void release0(long ptr);
    private native void destroy0(long ptr);
    private native void finalize0(long ptr);
}
