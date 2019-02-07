package org.qore.lang.qunit;

import org.qore.jni.QoreClosureMarker;

//! This class is used for test code taking no arguments
public interface TestCondition extends QoreClosureMarker {
    //! This method is called to call the test condition logic
    Object call() throws Throwable;
}
