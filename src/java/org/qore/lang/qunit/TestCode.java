package org.qore.lang.qunit;

import org.qore.jni.QoreClosureMarker;

//! This class is used for test code taking no arguments
public interface TestCode extends QoreClosureMarker {
    //! This method is called to run the test
    void call() throws Throwable;
}
