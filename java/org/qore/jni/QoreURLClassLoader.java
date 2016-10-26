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
    }

    public void addPathOrig(String path) throws Exception {
	//debugLog("QoreURLClassLoader.addPath(): file://" + path);
	super.addURL(new URL("file", null, 0, path));
    }

    synchronized protected Class findClass(String name) throws ClassNotFoundException {
	//debugLog("QoreURLClassLoader.findClass() " + name);

	Class result = classes.get(name); // checks in cached classes
        if (result != null) {
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

	throw new ClassNotFoundException(name);
    }

    public void addPath(String cp) {
	//System.out.println("addPath: " + cp);
	String seps = File.pathSeparator; // separators

	// want to accept both system separator and ';'
        if (!File.pathSeparator.equals(";")) {
	    seps+=";";
	}
	for (StringTokenizer st = new StringTokenizer(cp, seps, false); st.hasMoreTokens();) {
	    String pe = st.nextToken();
	    String bn = null;

	    if (pe.length() == 0) { continue; }

	    File fe = new File(pe);
	    if (fe.getName().indexOf('*') != -1) {
		bn = fe.getName();
		fe = fe.getParentFile();
		if (fe.getName().indexOf('*') != -1) {
		    errorLog("Ignoring wildcard in classpath directory element '" + pe + "'");
		    continue;
		}
            }

	    if (!fe.isAbsolute() && pe.charAt(0) != '/' && pe.charAt(0) != '\\') {
		fe = new File(fe.getPath());
	    }
	    try {
		fe = fe.getCanonicalFile();
	    }
	    catch (IOException thr) {
		errorLog("Ignoring non-existent classpath element '" + fe + "' (" + thr + ").");
		continue;
	    }
	    if (bn != null && !bn.isEmpty()) {
		fe = new File(fe, bn);
	    }
	    if (classPathElements.contains(fe.getPath())) {
		//errorLog("Skipping duplicate classpath element '"+fe+"'.");
		continue;
	    }
	    else {
		classPathElements.add(fe.getPath());
	    }

	    if (bn != null && !bn.isEmpty()) {
		debugLog("addJars() parent: " + fe.getParentFile().getPath() + " bn: " + bn);
		addJars(fe.getParentFile(),bn);
	    }
	    else if (!fe.exists()) { // s/never be due getCanonicalFile() above
		errorLog("Could not find classpath element '"+fe+"'");
	    }
	    else if (fe.isDirectory()) {
		addURL(createUrl(fe));
	    }
	    else if (fe.getName().toLowerCase().endsWith(".zip") || fe.getName().toLowerCase().endsWith(".jar")) {
		addURL(createUrl(fe));
	    }
	    else {
		errorLog("ClassPath element '"+fe+"' is not an existing directory and is not a file ending with '.zip' or '.jar'");
	    }
        }
	//infoLog("Class loader is using classpath: \""+classPath+"\".");
    }

    void infoLog(String msg) {
	System.out.println("QoreURLClassLoader INFO: " + msg);
    }

    void debugLog(String msg) {
	System.out.println("QoreURLClassLoader INFO: " + msg);
    }

    void errorLog(String msg) {
	System.out.println("QoreURLClassLoader ERROR: " + msg);
    }

    /**
     * Adds a set of JAR files using a generic base name to this loader's classpath.  See @link:addClassPath(String) for
     * details of the generic base name.
     */
    public void addJars(File dir, String nam) {
	if (nam.endsWith(".jar")) {
	    nam = nam.substring(0, (nam.length() - 4));
	}

	nam.replace(new StringBuffer("*"), new StringBuffer(".*"));

	if (!dir.exists()) {
	    errorLog("Cannot find directory for classpath element '" + dir + File.separator + nam + ".jar'");
	    return;
        }

	if (!dir.canRead()) {
	    errorLog("Cannot read directory for classpath element '" + dir + File.separator + nam + ".jar'");
	    return;
        }

	final String pattern = nam;

	File[] files = dir.listFiles(new FilenameFilter() {
		@Override
		public boolean accept(File dir, String name) {
		    return name.matches("^" + pattern + "\\.jar$");
		}
	    });

	if (files == null) {
	    errorLog("Error accessing directory for classpath element '" + dir + File.separator + nam + ".jar'");
	    return;
	}

	if (files.length == 0) {
	    return;
	}

	for (File f : files) {
	    addURL(createUrl(f));
	}
    }

    private URL createUrl(File fe) {
	try {
	    URL url = fe.toURI().toURL();
	    //infoLog("Added URL: '" + url.toString() + "'");
	    if (classPath.length()>0) {
		classPath += File.pathSeparator;
	    }
	    this.classPath+=fe.getPath();
	    return url;
	}
	catch (MalformedURLException thr) {
	    errorLog("classpath element '" + fe + "' could not be used to create a valid file system URL (" + thr + ")");
	    return null;
        }
    }
}
