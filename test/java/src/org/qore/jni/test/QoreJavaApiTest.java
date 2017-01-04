package org.qore.jni.test;

import org.qore.jni.*;
import java.util.HashMap;

class ThreadTest implements Runnable {
    public void run() {
        HashMap hm = (HashMap)QoreJavaApi.callFunction("get_qore_library_info");
    }
}


public class QoreJavaApiTest {
    static HashMap callFunctionTest() {
        HashMap hm = (HashMap)QoreJavaApi.callFunction("get_qore_library_info");

        (new Thread(new ThreadTest())).start();
        return hm;
    }
}
