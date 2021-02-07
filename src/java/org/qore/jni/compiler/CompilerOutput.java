package org.qore.jni.compiler;

import javax.tools.JavaFileObject;

/**
 * Contains the file data and the generated class for each compiled class
 */
public class CompilerOutput<T> {
    /**
     * The compiled class
     */
    public Class<T> cls;

    /**
     * The output file data
     */
    public JavaFileObject file;

    /**
     * Creates the object
     */
    public CompilerOutput(Class<T> cls, JavaFileObject file) {
        this.cls = cls;
        this.file = file;
    }
}
