
package org.qore.test;

import qore.Qore.OutputStream;
import qore.Qore.StringOutputStream;

import java.util.AbstractMap;

public class QoreDynamicTest4 extends OutputStream {
    private StringOutputStream stream = new StringOutputStream();

    public QoreDynamicTest4(String arg) throws Throwable {
        stream.write(arg.getBytes());
    }

    public void close() throws Throwable {
        stream.close();
    }

    public void write(byte[] data) throws Throwable {
        stream.write(data);
    }

    public String getData() throws Throwable {
        return stream.getData();
    }

    public static QoreDynamicTest4 test0(String arg) throws Throwable {
        return new QoreDynamicTest4(arg);
    }
}
