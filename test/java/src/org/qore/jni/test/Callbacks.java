package org.qore.jni.test;

public class Callbacks {

    static void callNow(Runnable r) {
        r.run();
    }

    static void callInThread(Runnable r) {
        new Thread(r).start();
//        try {Thread.sleep(1000);} catch (Exception e) {}
    }

}
