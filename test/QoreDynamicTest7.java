
package org.qore.test;

import qore.RecA;
import qore.RecB;
import qore.RecC;

public class QoreDynamicTest7 {
    public static RecA testA() throws Throwable {
        return new RecA();
    }

    public static RecB testB() throws Throwable {
        return new RecB();
    }

    public static RecC testC() throws Throwable {
        return new RecC();
    }
}
