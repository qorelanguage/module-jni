package org.qore.jni;

//! wrapper class for a %Qore object; this class holds a weak reference to the %Qore object
/** Due to the different in garbage collecting approaches (%Qore's garbage collector being
    <a href="https://github.com/qorelanguage/qore/wiki/Prompt-Collection">deterministic</a> and Java's not),
    strong references to %Qore objects must be managed outside of Java.

    @note API usage errors such as with releasing / deleting the object and then calling methods
    on the object will cause a crash

    @since 1.2
 */
public class QoreObjectBase {
    //! a pointer to the Qore object
    protected long obj;

    //! creates the wrapper object with a pointer to an object; this Java object holds a weak reference to the Qore object passed here
    public QoreObjectBase(long cptr, Object... args) throws Throwable {
        obj = create0(cptr, args);
    }

    //! creates the wrapper object with a pointer to an object; this Java object holds a weak reference to the Qore object passed here
    public QoreObjectBase(long obj) {
        this.obj = obj;
    }

    //! returns the pointer to the object
    public long get() {
        return obj;
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
    @SuppressWarnings("deprecation")
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

    private native long create0(long obj_ptr, Object... args);
    private native void release0(long obj_ptr);
    private native void destroy0(long obj_ptr);
    private native void finalize0(long obj_ptr);
}
