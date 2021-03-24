package org.qore.jni;

//! Helper class for pointer arguments to weak references Qore objects
/** This class is used as the constructor argument in order to not conflict with native arguments taking a Qore "int"

    @since 2.0
 */
public class QoreJavaObjectPtr {
    public long ptr;

    //! creates the wrapper object with a pointer to an object; this Java object holds a weak reference to the Qore object passed here
    public QoreJavaObjectPtr(long ptr) {
        this.ptr = ptr;
    }
}
