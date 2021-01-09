package org.qore.jni.compiler;

import java.util.List;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Collection;
import java.util.jar.JarEntry;
import java.util.HashMap;
import java.util.HashSet;

import java.io.IOException;
import java.io.File;

import java.nio.charset.StandardCharsets;

import java.net.URL;
import java.net.URLConnection;
import java.net.JarURLConnection;
import java.net.URI;
import java.net.URLDecoder;

import javax.tools.JavaFileObject;

import org.qore.jni.QoreURLClassLoader;

/**
 * based on source code by:
 * @author atamur
 * @since 15-Oct-2009
 */
class PackageInternalsFinder {
    private QoreURLClassLoader classLoader;
    private static final String CLASS_FILE_EXTENSION = ".class";

    private final HashMap<String, HashSet<String>> pendingClasses = new HashMap<String, HashSet<String>>();

    public PackageInternalsFinder(QoreURLClassLoader classLoader) {
        this.classLoader = classLoader;
    }

    public List<JavaFileObject> find(String packageName) throws IOException {
        ArrayList<JavaFileObject> result = new ArrayList<JavaFileObject>();

        if (QoreURLClassLoader.isDynamic(packageName)) {
            for (String bin_name : classLoader.getClassesInNamespace(packageName)) {
                result.add(new QoreJavaClassObject(bin_name, classLoader, QoreJavaClassObject.OT_NORMAL));
            }
        }

        for (String bin_name : classLoader.getPendingClassesForPackage(packageName)) {
            result.add(new QoreJavaClassObject(bin_name, classLoader, QoreJavaClassObject.OT_PENDING));
        }

        if (packageName.startsWith("org.qore.jni")) {
            for (String bin_name : classLoader.getInternalClassesForPackage(packageName)) {
                result.add(new QoreJavaClassObject(bin_name, classLoader, QoreJavaClassObject.OT_INTERNAL));
            }
        }

        //System.out.printf("PackageInternalsFinder.find(%s) 1 rv: %s\n", packageName, result);
        String javaPackageName = packageName.replaceAll("\\.", "/");

        //System.out.printf("PackageInternalsFinder.find(%s) checking %s\n", packageName, javaPackageName);
        for (URL url : classLoader.getURLs()) {
            result.addAll(listUnder(packageName, url));
        }

        //System.out.printf("PackageInternalsFinder.find(%s) 2 rv: %s\n", packageName, result.toString());
        return result;
    }

    private Collection<JavaFileObject> listUnder(String packageName, URL packageFolderURL) {
        File directory = new File(packageFolderURL.getFile());
        //System.out.printf("listUnder() packageName: %s url: %s dir: %s\n", packageName, packageFolderURL, directory.isDirectory());
        if (directory.isDirectory()) {
            // browse local .class files - useful for local execution
            return processDir(packageName, directory);
        } else { // browse a jar file
            return processJar(packageName, packageFolderURL);
        } // maybe there can be something else for more involved class loaders
    }

    private List<JavaFileObject> processJar(String packageName, URL packageFolderURL) {
        List<JavaFileObject> result = new ArrayList<JavaFileObject>();
        try {
            URLConnection conn = packageFolderURL.openConnection();
            //System.out.printf("jar type: %s\n", conn);
            if (!(conn instanceof JarURLConnection)) {
                // try to get a directory from the URL
                URL url = conn.getURL();
                //System.out.printf("URL: %s\n", conn.getURL());
                if (url.getProtocol().equals("file")) {
                    String path = URLDecoder.decode(url.getPath(), StandardCharsets.UTF_8);
                    File directory = new File(path);
                    //System.out.printf("path: %s\n", path);
                    if (directory.isDirectory()) {
                        return processDir(packageName, directory);
                    }
                }
                return result;
            }
            JarURLConnection jarConn = (JarURLConnection)conn;
            String rootEntryName = jarConn.getEntryName();
            if (rootEntryName == null) {
                rootEntryName = packageName.replaceAll("\\.", "/");
            }
            int rootEnd = rootEntryName.length() + 1;

            String jarUri = packageFolderURL.toExternalForm().split("!")[0];
            //System.out.printf("jarUri: %s (%s)\n", jarUri, packageFolderURL.toExternalForm());

            Enumeration<JarEntry> entryEnum = jarConn.getJarFile().entries();

            while (entryEnum.hasMoreElements()) {
                JarEntry jarEntry = entryEnum.nextElement();
                String name = jarEntry.getName();
                //System.out.printf("pkg: %s ren: %s name: %s\n", packageName, rootEntryName, name);
                if (name.startsWith(rootEntryName) && name.indexOf('/', rootEnd) == -1 && name.endsWith(CLASS_FILE_EXTENSION)) {
                    URI uri = URI.create(jarUri + "!/" + name);
                    String binaryName = name.replaceAll("/", ".");
                    binaryName = binaryName.replaceAll(CLASS_FILE_EXTENSION + "$", "");

                    //System.out.printf("adding pkg: %s url: %s bin: %s\n", packageName, uri, binaryName);
                    result.add(new CustomJavaFileObject(binaryName, uri));
                }
            }
        } catch (Exception e) {
            //e.printStackTrace();
            throw new RuntimeException("Wasn't able to open " + packageFolderURL + " as a jar file", e);
        }
        return result;
    }

    private List<JavaFileObject> processDir(String packageName, File directory) {
        List<JavaFileObject> result = new ArrayList<JavaFileObject>();

        File[] childFiles = directory.listFiles();
        for (File childFile : childFiles) {
            if (childFile.isFile()) {
                // We only want the .class files.
                if (childFile.getName().endsWith(CLASS_FILE_EXTENSION)) {
                    String binaryName = packageName + "." + childFile.getName();
                    binaryName = binaryName.replaceAll(CLASS_FILE_EXTENSION + "$", "");

                    result.add(new CustomJavaFileObject(binaryName, childFile.toURI()));
                }
            }
        }

        return result;
    }
}
