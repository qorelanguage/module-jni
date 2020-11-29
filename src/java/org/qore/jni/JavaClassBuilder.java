
package org.qore.jni;

import java.lang.reflect.Modifier;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;

import net.bytebuddy.ByteBuddy;
import net.bytebuddy.dynamic.DynamicType;
import net.bytebuddy.NamingStrategy;
import net.bytebuddy.implementation.MethodCall;
import net.bytebuddy.dynamic.scaffold.subclass.ConstructorStrategy;
import net.bytebuddy.description.type.TypeDescription;
import net.bytebuddy.dynamic.loading.ClassLoadingStrategy;

class JavaClassBuilder {
    private static Class objArray;

    private static final String CLASS_FIELD = "cls";

    // copied from org.objectweb.asm.Opcodes
    public static final int ACC_PUBLIC = (1 << 0);
    public static final int ACC_ABSTRACT = (1 << 10);

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

    static public DynamicType.Builder<?> getClassBuilder(String className, Class<?> parentClass, boolean is_abstract, long cptr)
            throws NoSuchMethodException {
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
            e.printStackTrace();
            throw new RuntimeException(String.format("qore-jni.jar module not in QORE_CLASSPATH; bytecode " +
                "generation unavailable; cannot perform dynamic imports in Java", e));
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

    static public Class<?> getClassFromBuilder(DynamicType.Builder<?> bb) {
        return bb
            .make()
            .load(JavaClassBuilder.class.getClassLoader(), ClassLoadingStrategy.Default.WRAPPER)
            .getLoaded();
    }
}