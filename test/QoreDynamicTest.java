
package org.qore.test;

import qore.Qore.Thread.Sequence;
import qore.Qore.StringOutputStream;
import qore.Qore.StreamWriter;
import qore.DynamicTest;

public class QoreDynamicTest {
    public static long test0() throws Throwable {
        Sequence seq = new Sequence();
        seq.next();
        return seq.getCurrent();
    }

    public static long test1(long start) throws Throwable {
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

    public static StringOutputStream test3() throws Throwable {
        StringOutputStream stream = new StringOutputStream();
        StreamWriter wr = new StreamWriter(stream);
        wr.printf("%s %s", "java", "test");
        return stream;
    }

    public static long test4() throws Throwable {
        return DynamicTest.test();
    }
}

interface InnerClassTest {
    public String get();
}