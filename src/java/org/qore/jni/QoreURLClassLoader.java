/**
    QoreURLClassLoader.java

    Qore Programming Language JNI Module

    Copyright (C) 2016 - 2021 Qore Technologies, s.r.o.

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
import java.io.InputStream;
import java.io.ByteArrayInputStream;

import java.nio.file.Files;
import java.nio.file.FileSystems;

import java.util.Hashtable;
import java.util.StringTokenizer;
import java.util.HashSet;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

//! Main ClassLoader for Java <-> %Qore and Java <-> Python integration
/** This ClassLoader supports dynamic imports from %Qore and Java using the following special packages:
    - <b><tt>python.</tt></b><i>[path...]</i>: indicates that the given path should be imported from Python to Java (after being
      imported to %Qore if necessary).
    - <b><tt>pythonmod.</tt></b><i>mod</i><tt>.</tt><i>[path...]</i>: indicates that the given path should be mapped to %Qore
      namespaces and/or classes after loading the Python module <i>mod</i> and importing into %Qore; the Java package
      segments after <tt><b>pythonmod.</b></tt><i>mod</i><tt>.</tt> are then converted to the equivalent %Qore namespace path
    - \c \b qore: indicates that the given path should be mapped to %Qore namespaces and/or classes; the Java package
      segments after <tt><b>qore.</b></tt> are then converted to the equivalent %Qore namespace path
    - <b><tt>qoremod.</tt></b><i>mod</i><tt>.</tt><i>[path...]</i>: indicates that the given path should be mapped to %Qore
      namespaces and/or classes after loading the %Qore module <i>mod</i>; the Java package
      segments after <tt><b>qoremod.</b></tt><i>mod</i><tt>.</tt> are then converted to the equivalent %Qore namespace path
 */
public class QoreURLClassLoader extends URLClassLoader {
    public static String INIT_PROP_NAME = "qore.QoreURLClassLoader.init";

    private static InheritableThreadLocal<QoreURLClassLoader> current =
        new InheritableThreadLocal<QoreURLClassLoader>();
    private HashSet<String> classPathElements = new HashSet<String>();
    private String classPath = new String();
    private long pgm_ptr = 0;
    private boolean enable_cache = false;
    // when used to bootstrap Java; cannot call getsystemClassLoader()
    private boolean bootstrap = false;
    // when used as the system class loader
    /** if true, we need to ensure that this object loads classes to make dynamic imports work
    */
    private boolean startup = false;

    //! for caching files during compilation
    private final HashMap<String, QoreJavaFileObject> classes = new HashMap<String, QoreJavaFileObject>();

    //! used to mark java class creation in progress; binary names used
    private HashSet<String> classInProgress = new HashSet<String>();

    //! cache of inner classes to resolve circular dependencies when injecting classes
    private HashMap<String, byte[]> pendingClasses = new HashMap<String, byte[]>();

    //! cache of classes when running as the boot classloader
    private HashMap<String, Class<?>> classCache = new HashMap<String, Class<?>>();

    //! static initialization
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

    //! constructor for using this class as the boot classloader
    public QoreURLClassLoader(ClassLoader parent) {
        super("QoreURLClassLoader", new URL[]{}, parent);
        enable_cache = true;
        setContextProgram(this);
        //System.out.printf("QoreURLClassLoader(ClassLoader parent: %s) this: %x (pgm: %x)\n",
        //    (parent == null ? "null" : parent.getClass().getCanonicalName()), hashCode(), pgm_ptr);

        startup = true;

        // set classpath from system classpath
        String cp = System.getProperty("java.class.path");
        if (cp != null && !cp.isEmpty()) {
            addPath(cp);
        }
    }

    //! constructor for using this class as the boot classloader for the module
    public QoreURLClassLoader() {
        super("QoreURLClassLoader", new URL[]{}, ClassLoader.getSystemClassLoader());
        setContext();
        enable_cache = true;
        setContextProgram(this);
        //System.out.printf("QoreURLClassLoader() this: %x (pgm: %x)\n", hashCode(), pgm_ptr);
    }

    //! constructor for using this class as the boot classloader for the module
    public QoreURLClassLoader(long p_ptr) {
        super("QoreURLClassLoader", new URL[]{}, ClassLoader.getSystemClassLoader());
        setContext();
        pgm_ptr = p_ptr;
        //System.out.printf("QoreURLClassLoader(long p_ptr: %x) this: %x\n", p_ptr, hashCode());
    }

    //! constructor with a QoreProgram pointer and a parent
    public QoreURLClassLoader(long p_ptr, ClassLoader parent) {
        super("QoreURLClassLoader", new URL[]{}, parent);
        // set the current classloader as the thread context classloader
        pgm_ptr = p_ptr;
        setContext();
        //System.out.printf("QoreURLClassLoader(long p_ptr: %x, ClassLoader parent: %s: %x) this: %x\n",
        //    p_ptr, (parent == null ? "null" : parent.getClass().getCanonicalName()),
        //    parent == null ? 0 : parent.hashCode(), hashCode());
    }

    //! constructor with a name and a parent
    public QoreURLClassLoader(String name, ClassLoader parent) {
        super(name, new URL[]{}, parent);
        setContext();
        enable_cache = true;
        setContextProgram(this);
        //System.out.printf("QoreURLClassLoader(name: '%s', ClassLoader parent: %s) this: %x (pgm: %x)\n",
        //    name, (parent == null ? "null" : parent.getClass().getCanonicalName()) + ")", hashCode(), pgm_ptr);
    }

    //! Sets the bootstrap flag
    public void setBootstrap() {
        bootstrap = true;
    }

    //! Clears the bootstrap flag
    public void clearBootstrap() {
        bootstrap = false;
    }

    /**
     * Add a class name/JavaFileObject mapping
     *
     * @param qualifiedClassName the name
     * @param javaFile           the file associated with the name
     */
    public void add(final String qualifiedClassName, final QoreJavaFileObject javaFile) {
        classes.put(qualifiedClassName, javaFile);
    }

    /**
     * @return An collection of QoreJavaFileObject instances for the classes in the
     * class loader.
     */
    public Collection<QoreJavaFileObject> files() {
        return Collections.unmodifiableCollection(classes.values());
    }

    @Override
    public InputStream getResourceAsStream(final String name) {
        if (name.endsWith(".class")) {
            String qualifiedClassName = name.substring(0,
                    name.length() - ".class".length()).replace('/', '.');
            QoreJavaFileObject file = classes.get(qualifiedClassName);
            if (file != null) {
                return new ByteArrayInputStream(file.getByteCode());
            }
        }
        return super.getResourceAsStream(name);
    }

    public void addPathOrig(String path) throws Exception {
        //debugLog("QoreURLClassLoader.addPath(): file://" + path);
        super.addURL(new URL("file", null, 0, path));
    }

    //! adds byte code for an inner class to the byte code cache; requires a binary name (ex: \c my.package.MyClass$1)
    public void addPendingClass(String bin_name, byte[] byte_code) {
        if (byte_code == null) {
            throw new RuntimeException("QoreURLClassLoader.addPendingClass() called with null byte_code");
        }
        pendingClasses.put(bin_name, byte_code);
        //System.out.printf("addPendingClass() this: %x '%s' len: %d hm size: %d\n", hashCode(), bin_name,
        //  byte_code.length, pendingClasses.size());
    }

    public Class<?> getResolveClass(String name) throws ClassNotFoundException {
        Class<?> rv = tryGetPendingClass(name);
        if (rv == null) {
            rv = super.findClass(name);
        }
        resolveClass(rv);
        //System.out.printf("returning resolved %s\n", name);
        return rv;
    }

    public void clearCache() {
        pendingClasses.clear();
    }

    public byte[] removePendingByteCode(String bin_name) {
        byte[] rv = pendingClasses.get(bin_name);
        return rv;
    }

    public ArrayList<String> getPendingClassesForPackage(String packageName) {
        ArrayList<String> rv = new ArrayList<String>();
        pendingClasses.forEach((k, v) -> {
            // see if binary name matches the package name
            int dot = k.indexOf(".");
            if (dot == -1 && packageName.isEmpty()
                || (dot > 0 && k.startsWith(packageName + ".") && k.indexOf(".", packageName.length() + 1) == -1)) {
                rv.add(k);
            }
        });
        //System.out.printf("getPendingClassesForPackage(%s) this: %x rv: %s (cache: %s)\n", packageName, hashCode(),
        //  rv, pendingClasses);
        return rv;
    }

    //! for resolving circular dependencies when defining inner classes
    private Class<?> tryGetPendingClass(String name) {
        byte[] byte_code = pendingClasses.remove(name);

        if (byte_code == null) {
            if (enable_cache) {
                Class<?> rv = classCache.remove(name);
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
        // create the class and return it
        return defineClassIntern(name, byte_code, 0, byte_code.length);
    }

    //! Supports generating classes from byte code as well as returning classes built in to the jni module
    protected Class<?> findClass(String bin_name) throws ClassNotFoundException {
        //System.out.printf("findClass() this: %x %s\n", hashCode(), bin_name);
        try {
        /*
        for (URL url : getURLs()) {
            System.out.printf("findClass(%s) this: %x elem: %s\n", bin_name, hashCode(), url.toString());
        }
        */

        Class<?> rv;

        if (bin_name.startsWith("net.bytebuddy.")) {
            byte[] byte_code = getCachedClass0(bin_name);
            //System.out.printf("findClass() %s: %s\n", bin_name, byte_code);
            if (byte_code != null) {
                //System.out.printf("findClass() %s returning cached\n", bin_name);
                return defineClassIntern(bin_name, byte_code, 0, byte_code.length);
            }
        } else if (bin_name.startsWith("org.qore.")) {
            byte[] byte_code = getInternalClass0(bin_name);
            //System.out.printf("findClass() %s: %s\n", bin_name, byte_code);
            if (byte_code != null) {
                //System.out.printf("findClass() %s returning internal\n", bin_name);
                return defineClassIntern(bin_name, byte_code, 0, byte_code.length);
            }
        }

        //System.out.printf("findClass() %s calling super\n", bin_name);
        //return super.findClass(bin_name);
        rv = super.findClass(bin_name);
        //System.out.printf("findClass() %x %s: returning super: %s\n", hashCode(), bin_name, rv);
        return rv;
        } catch (ClassNotFoundException e) {
            //e.printStackTrace();
            throw e;
        }
    }

    public byte[] getInternalClass(String bin_name) throws ClassNotFoundException {
        byte[] rv = getInternalClass0(bin_name);
        if (rv != null) {
            return rv;
        }
        throw new ClassNotFoundException(String.format("unknown internal class '%s'", bin_name));
    }

    public ArrayList<String> getInternalClassesForPackage(String packageName) {
        ArrayList<String> rv = new ArrayList<String>();
        getInternalClassesForPackage0(pgm_ptr, packageName, rv);
        //System.out.printf("getInternalClassesForPackage(%s) rv: '%s'\n", packageName, rv);
        return rv;
    }

    public synchronized boolean checkInProgress(String bin_name) {
        return classInProgress.contains(bin_name);
    }

    private synchronized boolean markInProgress(String bin_name) {
        if (classInProgress.contains(bin_name)) {
            return true;
        }
        classInProgress.add(bin_name);
        //debugLog("marked in progress " + bin_name);
        return false;
    }

    private synchronized void removeInProgress(String bin_name) {
        classInProgress.remove(bin_name);
        //debugLog("removed in progress " + bin_name);
    }

    public Class<?> loadResolveClass(String name) throws ClassNotFoundException {
        return loadClass(name);
    }

    // NOTE: loadClass(String, boolean) performs synchronization
    /**
     * Loads classes; returns pending classes injected by the jni module or the compiler
     */
    public Class<?> loadClass(String bin_name) throws ClassNotFoundException {
        //System.out.printf("QoreURLClassLoader.loadClass() this: %x '%s' pgm: %x (bootstrap: %s startup: %s)\n",
        //    hashCode(), bin_name, pgm_ptr, bootstrap, startup);
        Class<?> rv = findLoadedClass(bin_name);
        if (rv != null) {
            //System.out.printf("loadClass() %s returning loaded\n", bin_name);
            return rv;
        }

        if (bin_name.startsWith("java.")
            || bin_name.startsWith("javax.")
            || bin_name.startsWith("sun.")
            || bin_name.startsWith("org.qore.jni.")
            || bin_name.startsWith("org.qore.lang.")) {

            //return super.loadClass(bin_name);
            rv = super.loadClass(bin_name);
            //System.out.printf("loadClass() %s resolved %s with super: %s\n", bin_name, rv, rv.getClass().getClassLoader());
            return rv;
        }

        rv = tryGetPendingClass(bin_name);
        if (rv != null) {
            //System.out.printf("loadClass() %s returning pending\n", bin_name);
            return rv;
        }
        QoreJavaFileObject file = classes.get(bin_name);
        if (file != null) {
            byte[] bytes = file.getByteCode();
            //System.out.printf("loadClass() %s returning defineClass()\n", bin_name);
            return defineClass(bin_name, bytes, 0, bytes.length);
        }

        if (isDynamic(bin_name)) {
            // only remove from set if successful
            try {
                byte[] bytes = generateByteCode(bin_name);
                rv = defineClassIntern(bin_name, bytes, 0, bytes.length);
                //System.out.printf("loadClass() this: %x pgm: %x dyn %s returning generated %s\n", hashCode(),
                //    pgm_ptr, bin_name, rv);
                return rv;
            } catch (ClassNotFoundException e1) {
                //e1.printStackTrace();
                // block left empty on purpose
            } catch (RuntimeException e1) {
                //e1.printStackTrace();
                throw e1;
            } catch (Throwable e1) {
                //e1.printStackTrace();
                throw new RuntimeException(e1);
            }
        }

        if (!startup) {
            ClassLoader parent = getParent();
            if (parent == null) {
                parent = !bootstrap ? getSystemClassLoader() : getPlatformClassLoader();
            }
            if (parent != null) {
                try {
                    //return parent.loadClass(bin_name);
                    rv = parent.loadClass(bin_name);
                    //System.out.printf("loadClass() %s returning parent: %s\n", bin_name, rv);
                    return rv;
                } catch (ClassNotFoundException e) {
                    // ignore
                }
            }
        } else {
            try {
                // we must load classes first when we are a "startup" class loader, so that referenced dynamic
                // classes will be loadable
                rv = super.findClass(bin_name);
                if (rv != null) {
                    //System.out.printf("loadClass() %s returning super.findClass()\n", bin_name);
                    return rv;
                }
            } catch (ClassNotFoundException e) {
                // ignore
            }
        }
        //System.out.printf("loadClass() %s call super...\n", bin_name);
        return super.loadClass(bin_name);
    }

    //! Returns the Java class from the given Qore class with a Qore class ptr
    /**
     */
    public Class<?> loadClassWithPtr(String bin_name, long class_ptr) throws ClassNotFoundException {
        //System.out.printf("loadClassWithPtr() %s: %x\n", bin_name, class_ptr);
        Class<?> rv;
        synchronized(getClassLoadingLock(bin_name)) {
            rv = findLoadedClass(bin_name);
            if (rv != null) {
                //System.out.printf("loadClassWithPtr() %s returning loaded\n", bin_name);
                return rv;
            }
            rv = tryGetPendingClass(bin_name);
            if (rv != null) {
                //System.out.printf("loadClassWithPtr() %s returning pending\n", bin_name);
                return rv;
            }
            QoreJavaFileObject file = classes.get(bin_name);
            if (file != null) {
                byte[] bytes = file.getByteCode();
                //System.out.printf("loadClassWithPtr() %s returning defineClass()\n", bin_name);
                return defineClass(bin_name, bytes, 0, bytes.length);
            }

            // only remove from set if successful
            try {
                //System.out.printf("loadClassWithPtr() %s about to call generateByteCode(%s, %x)\n", bin_name, bin_name, class_ptr);
                byte[] bytes = generateByteCode(bin_name, class_ptr);
                rv = defineClassIntern(bin_name, bytes, 0, bytes.length);
                //System.out.printf("loadClassWithPtr() this: %x pgm: %x dyn %s returning generated %s\n", hashCode(),
                //    pgm_ptr, bin_name, rv);
            } catch (RuntimeException e1) {
                //e1.printStackTrace();
                throw e1;
            } catch (Throwable e1) {
                //e1.printStackTrace();
                throw new RuntimeException(e1);
            }
        }
        return rv;
    }

    //! Returns true if the given package name is dynamic
    static public boolean isDynamic(String bin_name) {
        return bin_name.equals("qore") || bin_name.equals("python")
            || (bin_name.startsWith("qore.") && bin_name.length() > 5)
            || (bin_name.startsWith("qoremod.") && bin_name.length() > 8)
            || (bin_name.startsWith("python.") && bin_name.length() > 7)
            || (bin_name.startsWith("pythonmod.") && bin_name.length() > 10);
    }

    protected Class<?> defineClassIntern(String name, byte[] byte_code, int off, int len) throws ClassFormatError {
        Class<?> rv = defineClass(name, byte_code, off, len);

        if (enable_cache) {
            classCache.put(name, rv);
            //debugLog("QoreURLClassLoader.defineClassIntern() caching " + name);
        } else {
            //debugLog("QoreURLClassLoader.defineClassIntern() not caching " + name);
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

    public long getPtr() {
        return pgm_ptr;
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

    //! Adds a path to the classpath
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
                //debugLog("adding jar: " + fileentry.getName() + " (" + fileentry.toString() + ")");
                addURL(createUrl(fileentry));
            } else {
                errorLog("ClassPath element '" + fileentry + "' is not an existing directory and is not a file " +
                    "ending with '.zip' or '.jar'");
            }
        }
        //infoLog("Class loader is using classpath: \"" + classPath + "\".");
    }

    //! Returns a list of classes in the given dynamic package
    public ArrayList<String> getClassesInNamespace(String packageName) {
        ArrayList<String> rv = new ArrayList<String>();
        ClassModInfo info = new ClassModInfo(packageName, true);
        getClassesInNamespace0(pgm_ptr, info.cls, info.mod, info.python, rv);
        //System.out.printf("getClassesInNamespace(%s) pgm: %x cls: '%s' mod: '%s' rv: %s\n", packageName, pgm_ptr,
        //  info.cls, info.mod, rv);

        if (rv.size() == 0) {
            ClassLoader parent = getParent();
            //System.out.printf("getClassesInNamespace() %s; parent is %s\n", packageName, parent.getClass().getName());
            if (parent instanceof QoreURLClassLoader) {
                return ((QoreURLClassLoader)parent).getClassesInNamespace(packageName);
            }
        }

        return rv;
    }

    public static ArrayList<File> splitClassPath(String classpath) {
        //debugLog("addPath: " + classpath);
        String seps = File.pathSeparator; // separators

        ArrayList<File> rv = new ArrayList<File>();

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
            rv.add(fileentry);
        }
        return rv;
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

    /**
     *  Clears the compilation cache after compiling
     */
    public void clearCompilationCache() {
        classCache.clear();
        clearCompilationCache0(pgm_ptr);
    }

    public synchronized byte[] generateByteCode(String bin_name) throws ClassNotFoundException {
        return generateByteCode(bin_name, 0);
    }

    public synchronized byte[] generateByteCode(String bin_name, long class_ptr) throws ClassNotFoundException {
        //System.out.printf("QoreURLClassLoader.generateByteCode() class: '%s' ptr: %x\n", bin_name, class_ptr);
        byte[] rv = pendingClasses.get(bin_name);
        if (rv == null) {
            if (markInProgress(bin_name)) {
                throw new ClassNotFoundException(String.format("%s is already being created", bin_name));
            }
            try {
                rv = generateByteCodeIntern(bin_name, class_ptr);
            } finally {
                removeInProgress(bin_name);
            }
        }
        //System.out.printf("generateByteCode() this: %x '%s': %s\n", hashCode(), bin_name, rv);
        return rv;
    }

    private byte[] generateByteCodeIntern(String bin_name, long class_ptr) throws ClassNotFoundException {
        ClassModInfo info = new ClassModInfo(bin_name);
        if (info.cls == null) {
            throw new ClassNotFoundException(String.format("invalid dynamic import path '%s'", bin_name));
        }

        byte[] rv = null;
        if (pgm_ptr != 0) {
            try {
                //System.out.printf("QoreURLClassLoader.generateByteCodeIntern() this: %x pgm: %x '%s' cptr: %x\n",
                //    hashCode(), pgm_ptr, bin_name, class_ptr);
                rv = generateByteCode0(pgm_ptr, info.cls, bin_name, info.mod, info.python, class_ptr);
                //System.out.printf("QoreURLClassLoader.generateByteCodeIntern() this: %x pgm: %x '%s' cptr: %x " +
                //    "rv: %s\n", hashCode(), pgm_ptr, bin_name, class_ptr, rv);
            } catch (ClassNotFoundException e) {
                throw e;
            } catch (RuntimeException e) {
                //e.printStackTrace();
                throw e;
            } catch (Throwable e) {
                //e.printStackTrace();
                throw new RuntimeException(e);
            }
        } else {
            //System.out.printf("QoreURLClassLoader.generateByteCodeIntern(%s) this: %x called with no Qore " +
            //    "program context", bin_name, hashCode());
        }
        if (rv == null) {
            throw new ClassNotFoundException(String.format("could not find a Qore source class matching '%s' to " +
                "create Java class '%s'", info.cls, bin_name));
        }
        // only put in the cache if the byte code is present
        pendingClasses.put(bin_name, rv);
        //System.out.printf("QoreURLClassLoader.generateByteCodeIntern() this: %x created/cached %s: %s\n",
        //    hashCode(), bin_name, rv.toString());
        return rv;
    }

    private URL createUrl(File fileentry) {
        try {
            URL url = fileentry.toURI().toURL();
            String path = url.getPath();
            if (url.getPath().endsWith(".jar")) {
                url = new URL("jar:file:" + path + "!/");
            }
            //infoLog("Added URL: '" + url.toString() + "'");
            if (classPath.length() > 0) {
                classPath += File.pathSeparator;
            }
            classPath += fileentry.getPath();
            return url;
        } catch (MalformedURLException thr) {
            errorLog("classpath element '" + fileentry + "' could not be used to create a valid file system URL (" +
                thr + ")");
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

    static private native byte[] getCachedClass0(String bin_name);
    static private native byte[] getInternalClass0(String name);
    private native byte[] generateByteCode0(long ptr, String qname, String name, String qore_module, boolean python,
        long class_ptr) throws Throwable;
    static private native void getClassesInNamespace0(long ptr, String packageName, String mod, boolean python,
        ArrayList<String> result);
    static private native void getInternalClassesForPackage0(long ptr, String packageName, ArrayList<String> result);
    static private native long getContextProgram0(QoreURLClassLoader syscl, BooleanWrapper created);
    static private native void shutdownContext0();
    static private native void clearCompilationCache0(long ptr);
    static private native void dummy0();
    static private native void debug0(long ptr);
}
