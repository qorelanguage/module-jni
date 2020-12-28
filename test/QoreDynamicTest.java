
package org.qore.test;

import qore.Qore.Thread.Sequence;

public class QoreDynamicTest {
    public static int test0() throws Throwable {
        Sequence seq = new Sequence();
        seq.next();
        return seq.getCurrent();
    }

    public static int test1(int start) throws Throwable {
        Sequence seq = new Sequence(start);
        seq.next();
        return seq.getCurrent();
    }

    public static String test2() {
        InnerClassTest t = new InnerClassTest() {
            public String get() {
                return "test";
            }
        };

        return t.get();
    }
}

interface InnerClassTest {
    public String get();
}