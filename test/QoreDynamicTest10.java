
package org.qore.test;

import pythonmod.xml.etree.ElementTree.*;

public class QoreDynamicTest10 extends ElementTree {
    public QoreDynamicTest10() throws Throwable {
    }

    public static Object test(String str, String name) throws Throwable {
        Element elem = (Element)pythonmod.xml.etree.ElementTree.$Functions.fromstring(str);
        Element e0 = (Element)elem.find(name);
        Object rv = e0.getMemberValue("text");
        return rv;
    }
}
