/*
    QoreURLClassLoader.java

    Qore Programming Language JNI Module

    Copyright (C) 2016 - 2020 Qore Technologies, s.r.o.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

package org.qore.jni;

import net.bytebuddy.ByteBuddy;
import net.bytebuddy.dynamic.DynamicType;
import net.bytebuddy.NamingStrategy;
import net.bytebuddy.implementation.MethodCall;
import net.bytebuddy.dynamic.scaffold.subclass.ConstructorStrategy;
import net.bytebuddy.description.type.TypeDescription;
import net.bytebuddy.dynamic.loading.ClassLoadingStrategy;

import java.net.URLClassLoader;
import java.net.URL;
import java.net.MalformedURLException;
import java.io.File;
import java.io.IOException;
import java.io.FilenameFilter;
import java.nio.file.Files;
import java.nio.file.FileSystems;
import java.util.Hashtable;
import java.util.StringTokenizer;
import java.util.HashSet;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

import java.lang.reflect.Modifier;
import java.lang.reflect.Type;

public class QoreURLClassLoader extends URLClassLoader {
    private static InheritableThreadLocal<QoreURLClassLoader> current = new InheritableThreadLocal<QoreURLClassLoader>();
    private HashSet<String> classPathElements = new HashSet<String>();
    private String classPath = new String();
    private long pgm_ptr = 0;
    private static Class objArray;

    private static final String CLASS_FIELD = "cls";

    // copied from org.objectweb.asm.Opcodes
    public static final int ACC_PUBLIC = (1 << 0);
    public static final int ACC_ABSTRACT = (1 << 10);

    // cache of inner classes to resolve circular dependencies when injecting classes
    private HashMap<String, byte[]> pendingClasses = new HashMap<String, byte[]>();

    // static initialization
    static {
        try {
            objArray = Class.forName("[L" + Object.class.getCanonicalName() + ";");

            /*
            Class<?>[] args = new Class<?>[2];
            args[0] = long.class;
            args[1] = objArray;
            mConstructorCall = BBTest.class.getDeclaredMethod("doConstructorCall", args);

            Class<?>[] args = new Class<?>[2];
            args[0] = String.class;
            args[1] = objArray;
            mStaticCall = BBTest.class.getDeclaredMethod("doStaticCall", args);

            args = new Class<?>[3];
            args[0] = String.class;
            args[1] = long.class;
            args[2] = objArray;
            mNormalCall = BBTest.class.getDeclaredMethod("doNormalCall", args);
            */
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    public QoreURLClassLoader(long p_ptr) {
        super("QoreURLClassLoader", new URL[]{}, ClassLoader.getPlatformClassLoader());
        // set the current classloader as the thread context classloader
        pgm_ptr = p_ptr;
        setContext();
    }

    public void addPathOrig(String path) throws Exception {
        //debugLog("QoreURLClassLoader.addPath(): file://" + path);
        super.addURL(new URL("file", null, 0, path));
    }

    // adds byte code for an inner class to the byte code cache; requires a dot name (ex: \c my.package.MyClass$1)
    public void addPendingClass(String name, byte[] byte_code) {
        //debugLog("addPendingClass: " + name + " len: " + byte_code.length);
        pendingClasses.put(name, byte_code);
    }

    // for resolving circular dependencies when defining inner classes
    private Class<?> tryGetPendingClass(String name) {
        byte[] byte_code = pendingClasses.get(name);
        if (byte_code == null) {
            return null;
        }
        // remove from the cache
        pendingClasses.remove(name);
        // create the class and return it
        return defineClass(name, byte_code, 0, byte_code.length);
    }

    protected Class<?> findClass(String name) throws ClassNotFoundException {
        //debugLog("findClass: " + name);
        /*
        debugLog("findClass: " + name);
        for (URL url : getURLs()) {
            debugLog(" + " + url.toString());
        }
        */
        Class<?> rv = tryGetPendingClass(name);
        if (rv != null) {
            return rv;
        }
        //return super.findClass(name);
        try {
            return super.findClass(name);
        } catch (ClassNotFoundException e) {
            if (name.startsWith("qore.") && name.length() > 5) {
                try {
                    return createJavaQoreClass(name);
                } catch (Throwable e1) {
                    e1.printStackTrace();
                    throw new ClassNotFoundException(e1.toString());
                }
            }
            throw e;
        }
    }

    /*
    public Class<?> loadClass(String name) throws ClassNotFoundException {
        debugLog("loadClass: " + name);
        for (URL url : getURLs()) {
            debugLog(" + " + url.toString());
        }
        Class<?> rv = tryGetPendingClass(name);
        if (rv != null) {
            return rv;
        }
        return super.loadClass(name);
    }
    */

    public Class<?> defineResolveClass(String name, byte[] b, int off, int len) throws ClassFormatError {
        Class<?> rv = defineClass(name, b, off, len);
        resolveClass(rv);
        return rv;
    }

    public static long getProgramPtr() {
        return current.get().pgm_ptr;
    }

    public static void setProgramPtr(long ptr) {
        current.get().pgm_ptr = ptr;
    }

    public static QoreURLClassLoader getCurrent() {
        return current.get();
    }

    // sets the current classloader as the thread's contextual class loader
    public void setContext() {
        Thread.currentThread().setContextClassLoader(this);
        current.set(this);
    }

    public void addPath(String classpath) {
        //debugLog("addPath: " + classpath);
        String seps = File.pathSeparator; // separators

        // want to accept both system separator and ';'
        if (!File.pathSeparator.equals(";")) {
            seps += ";";
        }
        for (StringTokenizer st = new StringTokenizer(classpath, seps, false); st.hasMoreTokens();) {
            String pathentry = st.nextToken();
            String basename = null;

            if (pathentry.length() == 0) {
                continue;
            }

            File fileentry = new File(pathentry);
            if (fileentry.getName().indexOf('*') != -1) {
                basename = fileentry.getName();
                fileentry = fileentry.getParentFile();
                if (fileentry.getName().indexOf('*') != -1) {
                    errorLog("Ignoring wildcard in classpath directory element '" + pathentry + "'");
                    continue;
                }
            }

            if (!fileentry.isAbsolute() && pathentry.charAt(0) != '/' && pathentry.charAt(0) != '\\') {
                fileentry = new File(fileentry.getPath());
            }
            try {
                fileentry = fileentry.getCanonicalFile();
            } catch (IOException thr) {
                errorLog("Ignoring non-existent classpath element '" + fileentry + "' (" + thr + ").");
                continue;
            }
            if (basename != null && !basename.isEmpty()) {
                fileentry = new File(fileentry, basename);
            }
            if (classPathElements.contains(fileentry.getPath())) {
                //errorLog("Skipping duplicate classpath element '" + fileentry + "'.");
                continue;
            } else {
                classPathElements.add(fileentry.getPath());
            }

            if (basename != null && !basename.isEmpty()) {
                //debugLog("addWildcard() parent: " + fileentry.getParentFile().getPath() + " basename: " + basename);
                addWildcard(fileentry.getParentFile(), basename);
            } else if (!fileentry.exists()) { // s/never be due getCanonicalFile() above
                errorLog("Could not find classpath element '" + fileentry + "'");
            } else if (fileentry.isDirectory()) {
                addURL(createUrl(fileentry));
            } else if (isLoadable(fileentry.getName())) {
                //infoLog("adding jar: " + fileentry.getName());
                addURL(createUrl(fileentry));
            } else {
                errorLog("ClassPath element '" + fileentry + "' is not an existing directory and is not a file ending with '.zip' or '.jar'");
            }
        }
        //infoLog("Class loader is using classpath: \"" + classPath + "\".");
    }

    static boolean isLoadable(String name) {
        name = name.toLowerCase();
        return (name.endsWith(".zip") || name.endsWith(".jar")) ? true : false;
    }

    static private void infoLog(String msg) {
        System.out.println("QoreURLClassLoader INFO: " + msg);
    }

    static private void debugLog(String msg) {
        System.out.println("QoreURLClassLoader DEBUG: " + msg);
    }

    static private void errorLog(String msg) {
        System.out.println("QoreURLClassLoader ERROR: " + msg);
    }

    /**
     * Adds a set of files using a generic base name to this loader's classpath.  See @link:addClassPath(String) for
     * details of the generic base name.
     */
    public void addWildcard(File dir, String nam) {
        if (!dir.exists()) {
            errorLog("Cannot find directory for classpath element '" + dir + File.separator + nam);
            return;
        }

        if (!dir.canRead()) {
            errorLog("Cannot read directory for classpath element '" + dir + File.separator + nam);
            return;
        }

        // make a regex pattern from the glob pattern
        final String pattern = nam.replace(new StringBuffer("*"), new StringBuffer(".*"));

        //debugLog("dir: " + dir + " pattern: " + pattern);

        File[] files = dir.listFiles(new FilenameFilter() {
            @Override
            public boolean accept(File dir, String name) {
                return name.matches("^" + pattern + "$");
            }
        });

        if (files == null) {
            errorLog("Error accessing directory for classpath element '" + dir + File.separator + nam);
            return;
        }

        if (files.length == 0) {
            debugLog("no matching files for classpath element '" + dir + File.separator + nam);
            return;
        }

        for (File f : files) {
            if (isLoadable(f.getName())) {
                //debugLog("adding file: " + f.getName());
                addURL(createUrl(f));
            }
        }
    }

    public DynamicType.Builder<?> getClassBuilder(String className, Class<?> parentClass, boolean is_abstract, long cptr)
            throws NoSuchMethodException, RuntimeException {
        DynamicType.Builder<?> bb;
        try {
            bb = new ByteBuddy()
                .with(new NamingStrategy.AbstractBase() {
                    @Override
                    public String name(TypeDescription superClass) {
                        return className;
                    }
                })
                .subclass(parentClass, ConstructorStrategy.Default.NO_CONSTRUCTORS);
        } catch (NoClassDefFoundError e) {
            throw new RuntimeException(String.format("qore-jni.jar module not in CLASSPATH; bytecode generation unavailable; " +
                "cannot perform dynamic imports in Java"));
        }

        int modifiers = ACC_PUBLIC;
        if (is_abstract) {
            modifiers |= ACC_ABSTRACT;
        }

        bb = bb.modifiers(modifiers);

        // add a static field for storing the class ptr
        bb = (DynamicType.Builder<?>)bb.defineField(CLASS_FIELD, long.class,
            Modifier.FINAL | Modifier.PUBLIC | Modifier.STATIC)
            .value(cptr);

        // create default constructor
        List<Type> paramTypes = new ArrayList<Type>();
        paramTypes.add(objArray);
        bb = (DynamicType.Builder<?>)bb.defineConstructor(Modifier.PUBLIC)
            .withParameters(paramTypes)
            .throwing(Throwable.class)
            .intercept(
                MethodCall.invoke(parentClass.getConstructor(long.class, objArray))
                .onSuper()
                .with(cptr)
                .withArgumentArray()
            )
            ;

        return bb;
    }

    public Class<?> getClassFromBuilder(DynamicType.Builder<?> bb) {
        return bb
            .make()
            .load(getClass().getClassLoader(), ClassLoadingStrategy.Default.WRAPPER)
            .getLoaded();
    }

    private Class<?> createJavaQoreClass(String name) throws ClassNotFoundException {
        String qname = "::" + name.substring(5).replace(".", "::");
        Class<?> jcls = createJavaQoreClass0(pgm_ptr, qname, name);
        if (jcls == null) {
            throw new ClassNotFoundException(String.format("could not find a Qore source class matching '%s' to create Java class '%s'",
                qname, name));
        }
        return jcls;
    }

    private URL createUrl(File fileentry) {
        try {
            URL url = fileentry.toURI().toURL();
            //infoLog("Added URL: '" + url.toString() + "'");
            if (classPath.length() > 0) {
                classPath += File.pathSeparator;
            }
            this.classPath += fileentry.getPath();
            return url;
        } catch (MalformedURLException thr) {
            errorLog("classpath element '" + fileentry + "' could not be used to create a valid file system URL (" + thr + ")");
            return null;
        }
    }

    static private native Class<?> createJavaQoreClass0(long ptr, String qname, String name);
}
