
package org.qore.test;

public interface QoreDynamicTest15 {
    public long get();
}

class TestClass {
    public long get(QoreDynamicTest15 test) {
        return test.get() + 1;
    }
}