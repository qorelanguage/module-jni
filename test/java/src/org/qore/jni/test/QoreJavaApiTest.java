package org.qore.jni.test;

import org.qore.jni.*;
import java.util.HashMap;
import java.time.ZonedDateTime;
import java.math.BigDecimal;

class ThreadTest implements Runnable {
    public void run() {
        try {
            HashMap hm = (HashMap)QoreJavaApi.callFunction("get_qore_library_info");
        } catch (Throwable e) {

        }
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
    static HashMap callFunctionTest() throws Throwable {
        HashMap hm = (HashMap)QoreJavaApi.callFunction("get_qore_library_info");

        (new Thread(new ThreadTest())).start();
        return hm;
    }

    static void threadTest(Runnable runme1, Runnable runme2) {
        (new Thread(new ThreadTest2(runme1, runme2))).start();
    }

    static ZonedDateTime dateTest(ZonedDateTime dt) {
        return dt.minusDays(2);
    }

    static String testObject1(QoreObject obj, String str) throws Throwable {
        try {
            return (String)obj.callMethod("getString", str);
        } finally {
            obj.release();
        }
    }

    static Object testObject2(QoreObject obj) throws Throwable {
        try {
            return obj.getMemberValue("member");
        } finally {
            obj.release();
        }
    }

    // do not release the object when returning it
    static QoreObject testObject3(QoreObject obj) {
        return obj;
    }

    static boolean testObject4(QoreObject obj, String cls) {
        try {
            return obj.instanceOf(cls);
        } finally {
            obj.release();
        }
    }

    static BigDecimal testObject5(String str) {
        return new BigDecimal(str);
    }

    static BigDecimal testObject5(BigDecimal num) {
        return num;
    }

    @SuppressWarnings("unchecked")
    static HashMap[] testObject6() {
        QoreHashMap[] hm = new QoreHashMap[2];
        hm[0] = new QoreHashMap();
        hm[0].put("1", 1);
        hm[1] = new QoreHashMap();
        hm[1].put("2", 2);

        return hm;
    }

    static HashMap testObject7(HashMap h) {
        h.remove("a");
        return h;
    }

    static Object testObject8(Object obj) {
        return obj;
    }

    static HashMap[] testObject9(HashMap[] l) {
        return l;
    }

    static HashMap[] testObject10(QoreObject obj) throws Throwable {
        try {
            return (HashMap[])obj.callMethod("getListOfHashes");
        } finally {
            obj.release();
        }
    }

    static String testObject11(QoreObject obj) {
        return obj.className();
    }
}
