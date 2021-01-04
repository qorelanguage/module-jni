
package org.qore.test;

import qore.Qore.ReadOnlyFile;

import java.util.AbstractMap;

public class QoreDynamicTest3 {
    public static AbstractMap test0(String arg) throws Throwable {
        return ReadOnlyFile.hstat(arg);
    }
}
