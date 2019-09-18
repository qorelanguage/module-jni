package org.qore.jni.test;

import org.qore.jni.*;

public class ClosureTest1 implements QoreClosureMarker {
    public String call(int i) {
        return "closure-" + String.valueOf(i);
    }
}
