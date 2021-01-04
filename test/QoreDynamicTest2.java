
package org.qore.test;

import java.util.Arrays;
import java.util.Iterator;

import qore.Qore.AbstractIterator;

public class QoreDynamicTest2 {
    public static AbstractIterator test0() throws Throwable {
        return new QoreIteratorTest(Arrays.asList("one", "two", "three").iterator());
    }
}

class QoreIteratorTest extends AbstractIterator {
    private Iterator i;

    QoreIteratorTest(Iterator i) throws Throwable {
        this.i = i;
    }

    public boolean next() {
        return i.hasNext();
    }

    public Object getValue() {
        return i.next();
    }

    public boolean valid() {
        return true;
    }
}
