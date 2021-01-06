package org.qore.jni.compiler;

import java.util.List;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Collection;
import java.util.jar.JarEntry;
import java.io.IOException;
import java.io.File;
import java.net.URL;
import java.net.URLConnection;
import java.net.JarURLConnection;
import java.net.URI;
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

    public PackageInternalsFinder(QoreURLClassLoader classLoader) {
        this.classLoader = classLoader;
    }

    public List<JavaFileObject> find(String packageName) throws IOException {
        ArrayList<JavaFileObject> result = new ArrayList<JavaFileObject>();

        if (QoreURLClassLoader.isDynamic(packageName)) {
            for (String bin_name : classLoader.getClassesInNamespace(packageName)) {
                result.add(new QoreJavaClassObject(bin_name, classLoader));
            }
        }

        //System.out.println("PackageInternalsFinder.find(" + packageName + ") rv: " + result);
        String javaPackageName = packageName.replaceAll("\\.", "/");

        Enumeration<URL> urlEnumeration = classLoader.getResources(javaPackageName);
        while (urlEnumeration.hasMoreElements()) {
            // one URL for each jar on the classpath that has the given package
            URL packageFolderURL = urlEnumeration.nextElement();
            result.addAll(listUnder(packageName, packageFolderURL));
        }

        //System.out.println("PackageInternalsFinder.find(" + packageName + ") rv: " + result.toString());
        return result;
    }

    private Collection<JavaFileObject> listUnder(String packageName, URL packageFolderURL) {
        File directory = new File(packageFolderURL.getFile());
        if (directory.isDirectory()) {
            // browse local .class files - useful for local execution
            return processDir(packageName, directory);
        } else { // browse a jar file
            return processJar(packageFolderURL);
        } // maybe there can be something else for more involved class loaders
    }

    private List<JavaFileObject> processJar(URL packageFolderURL) {
        List<JavaFileObject> result = new ArrayList<JavaFileObject>();
        try {
            String jarUri = packageFolderURL.toExternalForm().split("!")[0];

            URLConnection conn = packageFolderURL.openConnection();
            // in case of a local file, such as the compiled bytecode for classes put into a jar, the connection
            // returned may be of private type "sun.net.www.protocol.file.FileURLConnection"; in case it's not a
            // "JarURLConnection" object, we skip it without throwing an exception
            if (!(conn instanceof JarURLConnection)) {
                return result;
            }
            JarURLConnection jarConn = (JarURLConnection)conn;
            String rootEntryName = jarConn.getEntryName();
            int rootEnd = rootEntryName.length()+1;

            Enumeration<JarEntry> entryEnum = jarConn.getJarFile().entries();
            while (entryEnum.hasMoreElements()) {
                JarEntry jarEntry = entryEnum.nextElement();
                String name = jarEntry.getName();
                if (name.startsWith(rootEntryName) && name.indexOf('/', rootEnd) == -1 && name.endsWith(CLASS_FILE_EXTENSION)) {
                    URI uri = URI.create(jarUri + "!/" + name);
                    String binaryName = name.replaceAll("/", ".");
                    binaryName = binaryName.replaceAll(CLASS_FILE_EXTENSION + "$", "");

                    result.add(new CustomJavaFileObject(binaryName, uri));
                }
            }
        } catch (Exception e) {
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
