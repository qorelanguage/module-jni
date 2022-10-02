package org.qore.jni;

//! This marks a class that must have a \c call() method
/** The call() method can be declared with any argument and return types and will be called when this object is
 * converted to a Qore closure and the closure is called
*/
public interface QoreClosureMarkerImpl extends QoreClosureMarker {
    public abstract Object call(Object... args) throws Throwable;
}
