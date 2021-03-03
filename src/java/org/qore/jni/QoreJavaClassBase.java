package org.qore.jni;

//! marker base class for a %Qore classes imported dynamically into Java
/**
    @since 2.0
 */
public abstract class QoreJavaClassBase extends QoreObjectBase {
    //! Default constructor
    public QoreJavaClassBase(long cptr, long mptr, long vptr, Object... args) throws Throwable {
        super(cptr, mptr, vptr, args);
    }
}
