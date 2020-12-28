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

import java.net.URLClassLoader;
import java.net.MalformedURLException;
import java.net.URL;

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
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.Collections;

public class QoreURLClassLoader extends URLClassLoader {
    public static String INIT_PROP_NAME = "qore.QoreURLClassLoader.init";

    private static InheritableThreadLocal<QoreURLClassLoader> current =
        new InheritableThreadLocal<QoreURLClassLoader>();
    private HashSet<String> classPathElements = new HashSet<String>();
    private String classPath = new String();
    private long pgm_ptr = 0;
    private boolean enable_cache = false;

    // used to mark java class creation in progress; binary names used
    private static HashSet<String> classInProgress = new HashSet<String>();

    // cache of inner classes to resolve circular dependencies when injecting classes
    private HashMap<String, byte[]> pendingClasses = new HashMap<String, byte[]>();

    // cache of classes when running as the boot classloader
    private HashMap<String, Class<?>> classCache = new HashMap<String, Class<?>>();

    // cache of dynamically generated class info; binary name -> class data
    private static HashMap<String, QoreJavaDynamicClassData<?>> dynamicCache =
        new HashMap<String, QoreJavaDynamicClassData<?>>();

    // static initialization
    static {
        System.setProperty(INIT_PROP_NAME, "true");
        // loads and initializes the Qore library and the jni module (if necessary)
        try {
            dummy0();
        } catch (UnsatisfiedLinkError e0) {
            try {
                //debugLog("about to call initQore(): " + e0.toString());
                QoreJavaApi.initQore();
            } catch (ExceptionInInitializerError e1) {
                throw e1;
            } catch (Throwable e1) {
                throw new ExceptionInInitializerError(e1);
            }
        } catch (Throwable e0) {
            throw new ExceptionInInitializerError(e0);
        } finally {
            System.clearProperty(INIT_PROP_NAME);
        }
    }

    // constructor for using this class as the boot classloader
    public QoreURLClassLoader(ClassLoader parent) {
        super("QoreURLClassLoader", new URL[]{}, parent);
        enable_cache = true;
        setContextProgram(this);
        //debugLog("QoreURLClassLoader(ClassLoader parent: " + (parent == null ? "null" : parent.getClass().getCanonicalName()) + ")");
    }

    // constructor for using this class as the boot classloader for the module
    public QoreURLClassLoader() {
        super("QoreURLClassLoader", new URL[]{}, ClassLoader.getSystemClassLoader());
        setContext();
        enable_cache = true;
        setContextProgram(this);
        //debugLog("QoreURLClassLoader()");
    }

    // constructor for using this class as the boot classloader for the module
    public QoreURLClassLoader(long p_ptr) {
        super("QoreURLClassLoader", new URL[]{}, ClassLoader.getSystemClassLoader());
        setContext();
        pgm_ptr = p_ptr;
        //debugLog("QoreURLClassLoader(ptr: " + p_ptr + ")");
    }

    public QoreURLClassLoader(long p_ptr, ClassLoader parent) {
        super("QoreURLClassLoader", new URL[]{}, parent);
        // set the current classloader as the thread context classloader
        pgm_ptr = p_ptr;
        setContext();
        //debugLog("QoreURLClassLoader(long p_ptr = " + p_ptr + ", ClassLoader parent: " + (parent == null ? "null" : parent.getClass().getCanonicalName()) + ")");
    }

    public QoreURLClassLoader​(String name, ClassLoader parent) {
        super(name, new URL[]{}, parent);
        setContext();
        enable_cache = true;
        setContextProgram(this);
        //debugLog("QoreURLClassLoader(String name: " + name + ", ClassLoader parent: " + (parent == null ? "null" : parent.getClass().getCanonicalName()) + ")");
    }

    /*
    public URL findResource(final String name) {
        URL rv = super.findResource(name);
        System.out.printf("QoreURLClassLoader.findResource(%s): %s\n", name, rv);
        return rv;
    }

    public Enumeration<URL> findResources(final String name) throws IOException {
        Enumeration<URL> rv = super.findResources(name);
        System.out.printf("QoreURLClassLoader.findResources(%s): %s\n", name, rv);
        return rv;
    }

    public Enumeration<URL> getResources​(String name) throws IOException {
        Enumeration<URL> rv = super.getResources(name);
        System.out.println("getResources(" + name + ") rv: " + rv.toString());
        return rv;
    }
    */

    public void addPathOrig(String path) throws Exception {
        //debugLog("QoreURLClassLoader.addPath(): file://" + path);
        super.addURL(new URL("file", null, 0, path));
    }

    // adds byte code for an inner class to the byte code cache; requires a dot name (ex: \c my.package.MyClass$1)
    public void addPendingClass(String name, byte[] byte_code) {
        if (byte_code == null) {
            throw new RuntimeException("QoreURLClassLoader.addPendingClass() called with null byte_code");
        }
        pendingClasses.put(name, byte_code);
        //debugLog("addPendingClass: " + name + " len: " + byte_code.length + " hm size: " + pendingClasses.size());
    }

    public Class<?> getResolveClass(String name) throws ClassNotFoundException {
        Class<?> rv = tryGetPendingClass(name);
        if (rv == null) {
            rv = super.findClass(name);
        }
        resolveClass(rv);
        return rv;
    }

    public void clearCache() {
        pendingClasses.clear();
    }

    // for resolving circular dependencies when defining inner classes
    private Class<?> tryGetPendingClass(String name) {
        byte[] byte_code = pendingClasses.get(name);

        if (byte_code == null) {
            if (enable_cache) {
                Class<?> rv = classCache.get(name);
                if (rv == null) {
                    ClassLoader parent = getParent();
                    if (parent != null && (parent instanceof QoreURLClassLoader)) {
                        rv = ((QoreURLClassLoader)parent).tryGetPendingClass(name);
                    }
                }
                return rv;
            }

            return null;
        }
        // remove from the cache
        pendingClasses.remove(name);
        // create the class and return it
        return defineClassIntern(name, byte_code, 0, byte_code.length);
    }

    protected Class<?> findClass(String name) throws ClassNotFoundException {
        //debugLog("findClass: " + name);
        /*
        for (URL url : getURLs()) {
            debugLog(" + " + url.toString());
        }
        */
        Class<?> rv = tryGetPendingClass(name);
        if (rv != null) {
            return rv;
        }

        try {
            return super.findClass(name);
        } catch (ClassNotFoundException e) {
            //System.out.println("findClass() error: " + e.toString());
            if (name.startsWith("qore.") && name.length() > 5) {
                // check and set java class creation atomically
                if (markInProgress(name)) {
                    throw e;
                }

                // only remove from set if successful
                try {
                    rv = createJavaQoreClass(name, false).cls;
                } catch (RuntimeException e1) {
                    throw e1;
                } catch (Throwable e1) {
                    throw new RuntimeException(e1);
                }
                // remove marker from set atomically - only if the above call was successful
                removeInProgress(name);
                return rv;
            } else {
                byte[] byte_code;
                try {
                    byte_code = getCachedClass0(name);
                } catch (RuntimeException e1) {
                    throw e1;
                } catch (Throwable e1) {
                    throw new RuntimeException(e1);
                }
                if (byte_code != null) {
                    return defineClassIntern(name, byte_code, 0, byte_code.length);
                }
            }
            throw e;
        }
    }

    private synchronized boolean markInProgress(String name) {
        if (classInProgress.contains(name)) {
            return true;
        }
        classInProgress.add(name);
        //debugLog("marked in progress " + name);
        return false;
    }

    private synchronized void removeInProgress(String name) {
        classInProgress.remove(name);
        //debugLog("removed in progress " + name);
    }

    /*
    public Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        debugLog("loadClass: " + name + " resolve: " + resolve);
        return super.loadClass(name, resolve);
    }
    */

    public Class<?> loadClass(String name) throws ClassNotFoundException {
        //debugLog("loadClass: " + name);
        /*
        for (URL url : getURLs()) {
            debugLog(" + " + url.toString());
        }
        */
        Class<?> rv = tryGetPendingClass(name);
        if (rv != null) {
            return rv;
        }
        return super.loadClass(name);
    }

    protected Class<?> defineClassIntern(String name, byte[] byte_code, int off, int len) throws ClassFormatError {
        Class<?> rv = defineClass(name, byte_code, off, len);
        if (enable_cache) {
            classCache.put(name, rv);
            //debugLog("QoreURLClassLoader.defineClassIntern() caching " + name);
        }
        return rv;
    }

    public Class<?> defineClassUnconditional(String name, byte[] byte_code) throws ClassFormatError {
        // create the class and return it
        return defineClassIntern(name, byte_code, 0, byte_code.length);
    }

    public Class<?> defineResolveClass(String name, byte[] b, int off, int len) throws ClassFormatError {
        Class<?> rv = defineClassIntern(name, b, off, len);
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

    public ArrayList<String> getClassNamesInNamespace(String packageName) {
        ArrayList<String> rv = new ArrayList<String>();
        String qname = packageName.substring(4).replace(".", "::");
        getClassNamesInNamespace0(pgm_ptr, qname, rv);
        return rv;
        //URI uri = URI.create("file:" + packageName + ".Sequence");
        //result.add(new QoreJavaClassObject("qore.Qore.Thread.Sequence", classLoader));
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
            errorLog("Cannot find directory for classpath element '" + dir + File.separator + nam + "'");
            return;
        }

        if (!dir.canRead()) {
            errorLog("Cannot read directory for classpath element '" + dir + File.separator + nam + "'");
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
            errorLog("Error accessing directory for classpath element '" + dir + File.separator + nam + "'");
            return;
        }

        if (files.length == 0) {
            debugLog("no matching files for classpath element '" + dir + File.separator + nam + "'");
            return;
        }

        for (File f : files) {
            if (isLoadable(f.getName())) {
                //debugLog("adding file: " + f.getName());
                addURL(createUrl(f));
            }
        }
    }

    public synchronized QoreJavaDynamicClassData<?> createJavaQoreClass(String bin_name, boolean need_byte_code) throws ClassNotFoundException {
        //debugLog(String.format("QoreURLClassLoader.createJavaQoreClass() call: '%s' need_byte_code: %s", bin_name, need_byte_code));
        QoreJavaDynamicClassData<?> rv = dynamicCache.get(bin_name);
        if (rv == null) {
            markInProgress(bin_name);
            String qname = "::";
            if (bin_name.startsWith("qore.")) {
                qname += bin_name.substring(5);
            } else {
                qname += bin_name;
            }
            qname = qname.replaceAll("\\.", "::");
            //debugLog(String.format("QoreURLClassLoader.createJavaQoreClass() bin_name: %s qname: %s", bin_name, qname));
            if (pgm_ptr != 0) {
                try {
                    rv = createJavaQoreClass0(pgm_ptr, qname, bin_name, need_byte_code);
                } catch (RuntimeException e) {
                    throw e;
                } catch (Throwable e) {
                    throw new RuntimeException(e);
                }
            } else {
                debugLog("QoreURLClassLoader.createJavaQoreClass(" + bin_name + ") called with no Qore program context");
            }
            if (rv == null) {
                throw new ClassNotFoundException(String.format("could not find a Qore source class matching '%s' to " +
                    "create Java class '%s'", qname, bin_name));
            }
            dynamicCache.put(bin_name, rv);
            //debugLog(String.format("QoreURLClassLoader.createJavaQoreClass() created/cached %s: %s", bin_name, rv.toString()));
            // remove in progress only if successful
            removeInProgress(bin_name);
        } else {
            //debugLog(String.format("QoreURLClassLoader.createJavaQoreClass() got from cache %s: %s", bin_name, rv.toString()));
        }
        return rv;
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

    private void setContextProgram(QoreURLClassLoader new_syscl) {
        BooleanWrapper created = new BooleanWrapper();
        pgm_ptr = getContextProgram0(this, created);

        if (created.val) {
            Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    shutdownContext0();
                }
            });
        }
    }

    static private native byte[] getCachedClass0(String name);
    private native QoreJavaDynamicClassData<?> createJavaQoreClass0(long ptr, String qname, String name,
            boolean need_byte_code) throws Throwable;
    static private native void getClassNamesInNamespace0(long ptr, String packageName, ArrayList<String> result);
    static private native long getContextProgram0(QoreURLClassLoader syscl, BooleanWrapper created);
    static private native void shutdownContext0();
    static private native void dummy0();
}
