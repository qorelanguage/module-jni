
package org.qore.test;

import qore.ConcreteBase;
public class QoreDynamicTest8 extends ConcreteBase {
    public QoreDynamicTest8() throws Throwable {
    }

    public String test() {
        return "test";
    }

    public String test3() throws Throwable {
        return test2();
    }

    public String test5() throws Throwable {
        return test4();
    }
}
