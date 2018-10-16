package org.qore.jni;

//! This is meant to the be the wrapper class for Java classes wrapping Qore object classes
/** @note API usage errors such as with releasing / deleting the wrapped object and then calling methods
    on the object will cause a crash
 */
public class QoreObjectWrapper {
    //! the wrapper Qore object
    protected QoreObject obj;

    //! creates the wrapper object with the Qore object
    public QoreObjectWrapper(QoreObject obj) {
        this.obj = obj;
    }

    //! releases the Qore object; do not call any further methods on the object after this call
    public void release() {
        obj.release();
    }

    //! returns the Qore object
    public QoreObject get() {
        return obj;
    }

    //! returns the class name for the Qore object
    public String className() {
        return obj.className();
    }

    //! returns true if the object is an instance of the given class
    public boolean instanceOf(String class_name) {
        return obj.instanceOf(class_name);
    }
}
