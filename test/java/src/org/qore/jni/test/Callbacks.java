package org.qore.jni.test;

import java.lang.reflect.*;

public class Callbacks {

    static void callNow(Runnable r) {
        r.run();
    }

    static void callInThread(Runnable r) {
        new Thread(r).start();
    }

    static boolean trueIfThrows(Runnable r) {
        try {
            r.run();
            return false;
        } catch (MyException e) {
            return true;
        } catch (UndeclaredThrowableException e) {
            return ((InvocationTargetException)e.getCause()).getTargetException() instanceof MyException ? true : false;
        }
    }

    static void doThrow() {
        throw new MyException();
    }

    static String createString(StringFactory factory) {
        return "*" + factory.create() + "*";
    }

    private static class MyException extends RuntimeException {
    }
}
