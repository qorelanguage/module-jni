package org.qore.jni.compiler;

import java.io.*;
import java.util.*;
import javax.tools.*;

import org.qore.jni.QoreURLClassLoader;

public class QoreJavaFileManager implements JavaFileManager {
    private final QoreURLClassLoader classLoader;
    private final StandardJavaFileManager standardFileManager;
    private final PackageInternalsFinder finder;

    public QoreJavaFileManager(QoreURLClassLoader classLoader, StandardJavaFileManager standardFileManager) {
        this.classLoader = classLoader;
        this.standardFileManager = standardFileManager;
        finder = new PackageInternalsFinder(classLoader);
    }

    @Override
    public ClassLoader getClassLoader(Location location) {
        return classLoader;
    }

    @Override
    public String inferBinaryName(Location location, JavaFileObject file) {
        if (file instanceof QoreJavaClassObject) {
            return ((QoreJavaClassObject) file).binaryName();
        } else if (file instanceof CustomJavaFileObject) {
            return ((CustomJavaFileObject) file).binaryName();
        } else { // if it's not QoreJavaClassObject, then it's coming from standard file manager - let it handle the file
            return standardFileManager.inferBinaryName(location, file);
        }
    }

    @Override
    public boolean isSameFile(FileObject a, FileObject b) {
        return standardFileManager.isSameFile(a, b);
    }

    @Override
    public boolean handleOption(String current, Iterator<String> remaining) {
        return standardFileManager.handleOption(current, remaining);
    }

    @Override
    public boolean hasLocation(Location location) {
        // we don't care about source and other location types - not needed for compilation
        if (location == StandardLocation.CLASS_PATH || location == StandardLocation.PLATFORM_CLASS_PATH) {
            return true;
        }
        return false;
        //return standardFileManager.hasLocation(location);
    }

    @Override
    public Location getLocationForModule(Location location, String moduleName) throws IOException {
        return standardFileManager.getLocationForModule(location, moduleName);
    }

    @Override
    public Location getLocationForModule(Location location, JavaFileObject fo) throws IOException {
        return standardFileManager.getLocationForModule(location, fo);
    }

    @Override
    public Iterable<Set<Location>> listLocationsForModules(Location location) throws IOException {
        return standardFileManager.listLocationsForModules(location);
    }

    @Override
    public String inferModuleName(Location location) throws IOException {
        return standardFileManager.inferModuleName(location);
    }

    public void setLocation​(StandardLocation location, List<? extends File> files) throws IOException {
        standardFileManager.setLocation(location, files);
    }

    @Override
    public JavaFileObject getJavaFileForInput(Location location, String className, JavaFileObject.Kind kind) throws IOException {
        return standardFileManager.getJavaFileForInput(location, className, kind);
    }

    @Override
    public JavaFileObject getJavaFileForOutput(Location location, String className, JavaFileObject.Kind kind, FileObject sibling) throws IOException {
        return standardFileManager.getJavaFileForOutput(location, className, kind, sibling);
    }

    @Override
    public FileObject getFileForInput(Location location, String packageName, String relativeName) throws IOException {
        return standardFileManager.getFileForInput(location, packageName, relativeName);
    }

    @Override
    public FileObject getFileForOutput(Location location, String packageName, String relativeName, FileObject sibling) throws IOException {
        return standardFileManager.getFileForOutput(location, packageName, relativeName, sibling);
    }

    @Override
    public void flush() throws IOException {
        standardFileManager.flush();
    }

    @Override
    public void close() throws IOException {
        standardFileManager.close();
    }

    @Override
    public Iterable<JavaFileObject> list(Location location, String packageName, Set<JavaFileObject.Kind> kinds, boolean recurse) throws IOException {
        boolean baseModule = location.getName().equals("SYSTEM_MODULES[java.base]");
        if (baseModule || location == StandardLocation.PLATFORM_CLASS_PATH) {
            return standardFileManager.list(location, packageName, kinds, recurse);
        } else if (location == StandardLocation.CLASS_PATH && kinds.contains(JavaFileObject.Kind.CLASS)) {
            List<JavaFileObject> list = finder.find(packageName);
            if (list.size() > 0) {
                return list;
            }
            return standardFileManager.list(location, packageName, kinds, recurse);
        }
        return Collections.emptyList();
    }

    @Override
    public int isSupportedOption(String option) {
        return standardFileManager.isSupportedOption(option);
    }
}