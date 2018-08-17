package org.qore.jni.test;

import org.qore.jni.*;
import java.util.HashMap;

class ThreadTest implements Runnable {
    public void run() {
        HashMap hm = (HashMap)QoreJavaApi.callFunction("get_qore_library_info");
    }
}

class ThreadTest2 implements Runnable {
    public Runnable runme1;
    public Runnable runme2;

    ThreadTest2(Runnable runme1, Runnable runme2) {
        this.runme1 = runme1;
        this.runme2 = runme2;
    }

    public void run() {
        try {
            // this will throw an exception
            runme1.run();
        }
        catch (Exception e) {
        }

        runme2.run();
    }
}

public class QoreJavaApiTest {
    static HashMap callFunctionTest() {
        HashMap hm = (HashMap)QoreJavaApi.callFunction("get_qore_library_info");

        (new Thread(new ThreadTest())).start();
        return hm;
    }

    static void threadTest(Runnable runme1, Runnable runme2) {
        (new Thread(new ThreadTest2(runme1, runme2))).start();
    }
}
