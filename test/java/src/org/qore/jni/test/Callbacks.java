package org.qore.jni.test;

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
