package org.qore.jni.test;

import org.qore.jni.*;

import org.qore.lang.*;
import org.qore.lang.sqlutil.*;

import java.util.HashMap;
import java.time.ZonedDateTime;
import java.time.ZoneId;
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
        } catch (Exception e) {
        }

        runme2.run();
    }
}

class ThreadTest3 implements Runnable {
    static public String result;
    public QoreObject obj;

    ThreadTest3(QoreObject obj) {
        this.obj = obj;
    }

    public void run() {
        try {
            result = (String)obj.callMethod("getString");
        } catch (Throwable e) {
        }
    }
}

class ThreadTest4 implements Runnable {
    static public String result;

    public void run() {
        try {
            QoreObject obj = QoreJavaApi.newObjectSave("TestClass2");
            result = (String)obj.callMethod("getString");
        } catch (Throwable e) {
        }
    }
}

class ThreadTest5 implements Runnable {
    static public HashMap result;

    public void run() {
        try {
            result = (HashMap)QoreJavaApi.callStaticMethod("TestClass", "get", 1);
        } catch (Throwable e) {
        }
    }
}

class ClosureTest1 implements QoreClosureMarker {
    String call(int i) {
        return "closure-" + String.valueOf(i);
    }
}

class ClosureTest2 implements QoreClosureMarker {
    void call(int i) {
    }
}

class ClosureTest3 implements QoreClosureMarker {
    void call2(int i) {
    }
}

public class QoreJavaApiTest {
    static void throwTest() throws Throwable {
        throw new Throwable("test");
    }

    static HashMap callFunctionTest() throws Throwable {
        HashMap hm = (HashMap)QoreJavaApi.callFunction("get_qore_library_info");

        (new Thread(new ThreadTest())).start();
        return hm;
    }

    static Object callFunctionTest(String name) throws Throwable {
        return QoreJavaApi.callFunction(name);
    }

    static HashMap callStaticMethodTest() throws Throwable {
        return (HashMap)QoreJavaApi.callStaticMethod("TestClass", "get", 1);
    }

    static HashMap callStaticMethodTest2() throws InterruptedException {
        Thread mythread = new Thread(new ThreadTest5());
        mythread.start();
        mythread.join();
        return ThreadTest5.result;
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
        HashMap[] hm = new HashMap[2];
        hm[0] = new HashMap();
        hm[0].put("1", 1);
        hm[1] = new HashMap();
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

    static Object testObject12() throws Throwable {
        return QoreJavaApi.callFunction("get_object");
    }

    static Object testObject13() throws Throwable {
        return QoreJavaApi.callFunctionSave("get_object");
    }

    static String testObject14(QoreObject obj) throws InterruptedException {
        Thread mythread = new Thread(new ThreadTest3(obj));
        mythread.start();
        mythread.join();
        return ThreadTest3.result;
    }

    static String testObject15() throws InterruptedException {
        Thread mythread = new Thread(new ThreadTest4());
        mythread.start();
        mythread.join();
        return ThreadTest4.result;
    }

    static int[] testDate1(QoreRelativeTime dt) {
        int[] rv = new int[7];
        rv[0] = dt.year;
        rv[1] = dt.month;
        rv[2] = dt.day;
        rv[3] = dt.hour;
        rv[4] = dt.minute;
        rv[5] = dt.second;
        rv[6] = dt.us;
        return rv;
    }

    static QoreRelativeTime testDate2(QoreRelativeTime dt) {
        return dt;
    }

    static QoreClosureMarker getClosure1() {
        return new ClosureTest1();
    }

    static QoreClosureMarker getClosure2() {
        return new ClosureTest2();
    }

    static QoreClosureMarker getClosure3() {
        return new ClosureTest3();
    }

    @SuppressWarnings("unchecked")
    static HashMap<String, Object>[] getCallStack() throws Throwable {
        return (HashMap<String, Object>[])QoreJavaApi.callFunction("gtcs", 50);
    }

    static void testException(String err, String desc, Object arg) throws QoreException {
        throw new QoreException(err, desc, arg);
    }

    static void testException(String err, String desc) throws QoreException {
        throw new QoreException(err, desc);
    }
}
