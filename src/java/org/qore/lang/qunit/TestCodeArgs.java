package org.qore.lang.qunit;

import org.qore.jni.QoreClosureMarker;

//! This class is used for test code taking an arbitrary number of arguments
public interface TestCodeArgs extends QoreClosureMarker {
    //! This method is called to run the test
    void call(Object... args) throws Throwable;
}
