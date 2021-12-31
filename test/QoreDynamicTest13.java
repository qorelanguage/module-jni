
package org.qore.test;

import qoremod.RestClient.RestClient;

import org.qore.jni.Hash;

public class QoreDynamicTest13 {
    public static RestClient test0(Hash opts) throws Throwable {
        return new RestClient(opts, true);
    }
}
