
package org.qore.test;

import pythonmod.os.$Functions;

public class QoreDynamicTest11 {
    public QoreDynamicTest11() throws Throwable {
    }

    public String[] test() throws Throwable {
        return (String[])pythonmod.os.$Functions.get_exec_path();
    }
}
