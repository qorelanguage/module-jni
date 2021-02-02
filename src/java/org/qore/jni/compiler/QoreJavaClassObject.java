package org.qore.jni.compiler;

import java.net.URI;
import java.io.*;
import javax.tools.JavaFileObject;
import javax.lang.model.element.NestingKind;
import javax.lang.model.element.Modifier;

import org.qore.jni.QoreURLClassLoader;

/**
 * based on source code by:
 * @author atamur
 * @since 15-Oct-2009
 */
class QoreJavaClassObject implements JavaFileObject {
    public static final int OT_NORMAL = 0;
    public static final int OT_PENDING = 1;
    public static final int OT_INTERNAL = 2;

    private final String binaryName;
    private final QoreURLClassLoader classLoader;
    private final int type;

    public QoreJavaClassObject(String binaryName, QoreURLClassLoader classLoader, int type) {
        this.binaryName = binaryName;
        this.classLoader = classLoader;
        this.type = type;
    }

    @Override
    public URI toUri() {
        throw new UnsupportedOperationException();
    }

    @Override
    public InputStream openInputStream() throws IOException {
        //System.out.printf("openInputStream: %s cl: %s (type: %d)\n", binaryName, classLoader, type);
        try {
            byte[] byte_code;
            if (type == OT_PENDING) {
                byte_code = classLoader.removePendingByteCode(binaryName);
            } else if (type == OT_INTERNAL) {
                byte_code = classLoader.getInternalClass(binaryName);
            } else {
                byte_code = classLoader.generateByteCode(binaryName);
            }
            //System.out.printf("openInputStream: '%s': got %d bytes\n", binaryName,
            //  byte_code == null ? -1 : byte_code.length);
            return new ByteArrayInputStream(byte_code);
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public OutputStream openOutputStream() throws IOException {
        throw new UnsupportedOperationException();
    }

    @Override
    public String getName() {
        return binaryName;
    }

    @Override
    public Reader openReader(boolean ignoreEncodingErrors) throws IOException {
        throw new UnsupportedOperationException();
    }

    @Override
    public CharSequence getCharContent(boolean ignoreEncodingErrors) throws IOException {
        throw new UnsupportedOperationException();
    }

    @Override
    public Writer openWriter() throws IOException {
        throw new UnsupportedOperationException();
    }

    @Override
    public long getLastModified() {
        return 0;
    }

    @Override
    public boolean delete() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Kind getKind() {
        return Kind.CLASS;
    }

    @Override // copied from SimpleJavaFileManager
    public boolean isNameCompatible(String simpleName, Kind kind) {
        String baseName = simpleName + kind.extension;
        return kind.equals(getKind())
            && (baseName.equals(getName())
            || getName().endsWith("/" + baseName));
    }

    @Override
    public NestingKind getNestingKind() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Modifier getAccessLevel() {
        throw new UnsupportedOperationException();
    }

    public String binaryName() {
        return binaryName;
    }

    @Override
    public String toString() {
        return String.format("QoreJavaClassObject{binaryName=%s,type=%s}", binaryName,
            type == OT_NORMAL ? "NORMAL" : (type == OT_PENDING ? "PENDING" : "INTERNAL"));
    }
}
