package org.qore.lang;

import org.qore.jni.QoreClosureMarker;

//! This class is used for row code callbacks with bulk SQL classes
public interface LogCallback extends QoreClosureMarker {
    //! This method is called to provide logging information
    void call(String fmt, Object... args);
}
