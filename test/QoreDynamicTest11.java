
package org.qore.test;

import python.os.$Functions;

public class QoreDynamicTest11 extends ElementTree {
    public QoreDynamicTest11() throws Throwable {
    }

    public String test() throws Throwable {
        return (String)python.os.$Functions.get_exec_path();
    }
}
