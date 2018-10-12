package org.qore.jni;

import java.util.HashMap;
import java.util.Map;

public class QoreHashMap<K,V> extends HashMap<K,V> {
    public QoreHashMap() {
        super();
    }

    public QoreHashMap(Map<K,V> m) {
        super(m);
    }
}
