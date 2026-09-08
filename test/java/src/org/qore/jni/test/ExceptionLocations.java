// Copyright (C) 2026 Qore Technologies, s.r.o.
package org.qore.jni.test;

/** Supplies deterministic Java exception locations for the JNI conversion tests. */
public final class ExceptionLocations {
    private ExceptionLocations() {
    }

    public static void throwWithLocation(String filename, int line) {
        IllegalStateException exception = new IllegalStateException("location fixture");
        exception.setStackTrace(new StackTraceElement[] {
            new StackTraceElement("fixture.Top", "throwHere", filename, line),
            new StackTraceElement("fixture.Caller", "callHere", filename, line),
            new StackTraceElement("fixture.Entry", "enterHere", "Entry.java", 123),
        });
        throw exception;
    }

    public static void throwWithoutStack() {
        IllegalStateException exception = new IllegalStateException("empty stack fixture");
        exception.setStackTrace(new StackTraceElement[0]);
        throw exception;
    }
}
