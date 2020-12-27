
package org.qore.test;

import qore.Qore.Thread.Sequence;

public class QoreDynamicTest {
    public static int test0() throws Throwable {
        Sequence seq = new Sequence();
        seq.next();
        return seq.getCurrent();
    }

    public static void test1() {
        /*
        LambdaTest t = () -> {
            return "test";
        };
        */

        LambdaTest t = new LambdaTest() {
            public String get() {
                return "test";
            }
        };

        System.out.println(t.get());
    }
}

interface LambdaTest {
    public String get();
}