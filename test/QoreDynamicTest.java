
package org.qore.test;

import qore.Qore.Thread.Sequence;
import qore.Qore.StringOutputStream;
import qore.Qore.StreamWriter;

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

    public static StringOutputStream test3() throws Throwable {
        StringOutputStream stream = new StringOutputStream();
        StreamWriter wr = new StreamWriter(stream);
        wr.printf("%s %s", "java", "test");
        return stream;
    }
}

interface InnerClassTest {
    public String get();
}