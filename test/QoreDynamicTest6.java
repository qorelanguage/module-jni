
package org.qore.test;

import qoremod.xml.XmlRpcClient;

import org.qore.jni.Hash;

public class QoreDynamicTest6 {
    public static XmlRpcClient test0(Hash opts) throws Throwable {
        return new XmlRpcClient(opts, true);
    }
}
