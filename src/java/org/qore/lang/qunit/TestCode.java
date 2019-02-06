package org.qore.lang.qunit;

import org.qore.jni.QoreClosureMarker;

//! This class is used for row code callbacks with bulk SQL classes
public interface TestCode extends QoreClosureMarker {
    //! This method is called to run the test
    void call() throws Throwable;
}
