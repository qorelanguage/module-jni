

import qore.Qore.Thread.Sequence;

public class QoreDynamicTest {
    public static int test0() throws Throwable {
        Sequence seq = new Sequence();
        seq.next();
        return seq.getCurrent();
    }
}
