package org.qore.jni;

public class QoreJavaDynamicClassData<T> {
    /**
     * The compiled class
     */
    public Class<T> cls;

    /**
     * The byte code for the generated class
     */
    public byte[] byte_code;

    /**
     * Creates the object
     */
    public QoreJavaDynamicClassData(Class<T> cls, byte[] byte_code) {
        this.cls = cls;
        this.byte_code = byte_code;
    }

    @Override
    public String toString() {
        return "QoreJavaDynamicClassData{" +
            "cls=" + (cls != null ? cls.toString() : "n/a") +
            ", byte_code=" + (byte_code != null ? byte_code.length : 0) + " bytes" +
            '}';
    }
}
