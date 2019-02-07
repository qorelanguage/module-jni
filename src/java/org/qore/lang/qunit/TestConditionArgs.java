package org.qore.lang.qunit;

import org.qore.jni.QoreClosureMarker;

//! This class is used for test code taking an arbitrary number of arguments
public interface TestConditionArgs extends QoreClosureMarker {
    //! This method is called to call the test condition logic
    Object call(Object... args) throws Throwable;
}
