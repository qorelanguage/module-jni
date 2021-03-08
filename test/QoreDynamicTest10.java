
package org.qore.test;

import pythonmod.xml.etree.ElementTree.$Functions;
//import pythonmod.xml.etree.ElementTree.ElementTree;
//import pythonmod.xml.etree.ElementTree.*;
import qore.Python.xml.etree.ElementTree.*;
import org.qore.jni.Hash;
import org.qore.jni.QoreObject;

public class QoreDynamicTest10 extends ElementTree {
    public QoreDynamicTest10() throws Throwable {
    }

    public Object test(String str, String name) throws Throwable {
        Element elem = (Element)pythonmod.xml.etree.ElementTree.$Functions.fromstring(str);
        Element e0 = (Element)elem.find(name);
        Object rv = e0.getMemberValue("text");
        return rv;
    }
}
