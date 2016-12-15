package org.qore.jni;

import java.net.URLClassLoader;
import java.net.URL;
import java.io.File;
import java.util.StringTokenizer;
import java.io.IOException;
import java.util.HashSet;
import java.util.Arrays;
import java.net.MalformedURLException;
import java.nio.file.Files;
import java.nio.file.FileSystems;
import java.util.Hashtable;
import java.io.FilenameFilter;

public class QoreURLClassLoader extends URLClassLoader {
    private HashSet<String> classPathElements = new HashSet<String>();
    private String classPath = new String();
    private Hashtable<String, Class> classes = new Hashtable<String, Class>(); //used to cache already defined classes

    public QoreURLClassLoader() {
	super(((URLClassLoader)ClassLoader.getSystemClassLoader()).getURLs());
	// set the current classloader as the thread context classloader
	setContext();
    }

    public void addPathOrig(String path) throws Exception {
	//debugLog("QoreURLClassLoader.addPath(): file://" + path);
	super.addURL(new URL("file", null, 0, path));
    }

    /*
    protected Class loadClass(String name, boolean resolve) throws ClassNotFoundException {
	debugLog("loadClass: " + name + " resolve: " + resolve);
	try {
	    Class rv = super.loadClass(name, resolve);
	    debugLog("got: " + rv);
	    return rv;
	}
	catch (ClassNotFoundException e) {
	    debugLog("no class found");
	    throw e;
	}
    }
    */

    // sets the current classloader as the thread's contextual class loader
    public void setContext() {
	Thread.currentThread().setContextClassLoader(this);
    }

    synchronized protected Class findClass(String name) throws ClassNotFoundException {
	//debugLog("QoreURLClassLoader.findClass() " + name);

	Class result = classes.get(name); // checks in cached classes
	if (result != null) {
	    //debugLog("findClass() FOUND " + name + " in cache");
	    return result;
	}

	String pname = name.replace('.', File.separatorChar);
	for (String path : classPathElements) {
	    path = path + File.separator + pname + ".class";
	    File f = new File(path);
	    if (f.exists() && f.isFile()) {
		//debugLog("LOADING: " + path);
		try {
		    byte[] data = Files.readAllBytes(FileSystems.getDefault().getPath(path));
		    result = defineClass(name, data, 0, data.length);
		    classes.put(name, result);
		    //debugLog("RETURNING: " + result + " len: " + data.length);
		    return result;
		}
		catch (IOException e) {
		    errorLog("IOException: " + e);
		    // ignore
		}
	    }
	}

	//debugLog("QoreURLClassLoader.findClass() " + name);
	return super.findClass(name);
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
	    }
	    catch (IOException thr) {
		errorLog("Ignoring non-existent classpath element '" + fileentry + "' (" + thr + ").");
		continue;
	    }
	    if (basename != null && !basename.isEmpty()) {
		fileentry = new File(fileentry, basename);
	    }
	    if (classPathElements.contains(fileentry.getPath())) {
		//errorLog("Skipping duplicate classpath element '" + fileentry + "'.");
		continue;
	    }
	    else {
		classPathElements.add(fileentry.getPath());
	    }

	    if (basename != null && !basename.isEmpty()) {
		//debugLog("addWildcard() parent: " + fileentry.getParentFile().getPath() + " basename: " + basename);
		addWildcard(fileentry.getParentFile(), basename);
	    }
	    else if (!fileentry.exists()) { // s/never be due getCanonicalFile() above
		errorLog("Could not find classpath element '" + fileentry + "'");
	    }
	    else if (fileentry.isDirectory()) {
		addURL(createUrl(fileentry));
	    }
	    else if (isLoadable(fileentry.getName())) {
		//infoLog("adding jar: " + fileentry.getName());
		addURL(createUrl(fileentry));
            }
	    else {
		errorLog("ClassPath element '" + fileentry + "' is not an existing directory and is not a file ending with '.zip', '.jar', or '.class'");
	    }
        }
	//infoLog("Class loader is using classpath: \"" + classPath + "\".");
    }

    static boolean isLoadable(String name) {
        name = name.toLowerCase();
        return (name.endsWith(".zip") || name.endsWith(".jar") || name.endsWith(".class")) ? true : false;
    }

    void infoLog(String msg) {
	System.out.println("QoreURLClassLoader INFO: " + msg);
    }

    void debugLog(String msg) {
	System.out.println("QoreURLClassLoader DEBUG: " + msg);
    }

    void errorLog(String msg) {
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

    private URL createUrl(File fileentry) {
	try {
	    URL url = fileentry.toURI().toURL();
	    //infoLog("Added URL: '" + url.toString() + "'");
	    if (classPath.length() > 0) {
		classPath += File.pathSeparator;
	    }
	    this.classPath += fileentry.getPath();
	    return url;
	}
	catch (MalformedURLException thr) {
	    errorLog("classpath element '" + fileentry + "' could not be used to create a valid file system URL (" + thr + ")");
	    return null;
        }
    }
}
