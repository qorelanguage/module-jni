
package org.qore.test;

public interface QoreDynamicTest16 {
    public void messageArrived(String topic, Class c);
}

class TestClass {
    public void get(QoreDynamicTest16 test) {
        test.messageArrived("test", null);
    }
}