package org.qore.jni.compiler;

import java.net.URI;
import java.io.*;
import javax.tools.JavaFileObject;
import javax.lang.model.element.NestingKind;
import javax.lang.model.element.Modifier;

import org.qore.jni.QoreURLClassLoader;
import org.qore.jni.QoreJavaDynamicClassData;

/**
 * based on source code by:
 * @author atamur
 * @since 15-Oct-2009
 */
class QoreJavaClassObject implements JavaFileObject {
    private final String binaryName;
    private final QoreURLClassLoader classLoader;

    public QoreJavaClassObject(String binaryName, QoreURLClassLoader classLoader) {
        this.binaryName = binaryName;
        this.classLoader = classLoader;
    }

    @Override
    public URI toUri() {
        throw new UnsupportedOperationException();
    }

    @Override
    public InputStream openInputStream() throws IOException {
        //System.out.println("openInputStream: " + binaryName + " cl: " + classLoader);
        try {
            byte[] byte_code = classLoader.createJavaQoreClass(binaryName, true).byte_code;
            //System.out.println("openInputStream: " + binaryName + ": got " + byte_code.length + " bytes");
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

}
