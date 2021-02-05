
package org.qore.jni;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Type;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;
import java.util.Collections;

import java.nio.file.Files;
import java.nio.file.Path;

import net.bytebuddy.ByteBuddy;
import net.bytebuddy.description.modifier.Ownership;
import net.bytebuddy.description.modifier.Visibility;
import net.bytebuddy.description.type.TypeDescription;
import net.bytebuddy.description.type.TypeDefinition;
import net.bytebuddy.description.modifier.MethodArguments;
import net.bytebuddy.description.method.MethodDescription.Token;
import net.bytebuddy.dynamic.DynamicType;
import net.bytebuddy.dynamic.scaffold.subclass.ConstructorStrategy;
import net.bytebuddy.dynamic.loading.ClassLoadingStrategy;
import net.bytebuddy.dynamic.scaffold.InstrumentedType;
import net.bytebuddy.dynamic.scaffold.TypeValidation;
import net.bytebuddy.implementation.bind.annotation.Argument;
import net.bytebuddy.implementation.bind.annotation.RuntimeType;
import net.bytebuddy.implementation.bytecode.assign.Assigner;
import net.bytebuddy.implementation.MethodCall;
import net.bytebuddy.implementation.FixedValue;
import net.bytebuddy.NamingStrategy;

import org.qore.jni.QoreURLClassLoader;

public class JavaClassBuilder {
    private static Class objArray;
    private static Method mStaticCall;
    private static Method mNormalCall;
    private static final String CLASS_FIELD = "qore_cls_ptr";

    // copied from org.objectweb.asm.Opcodes
    public static final int ACC_PUBLIC    = (1 << 0);
    public static final int ACC_PRIVATE   = (1 << 1);
    public static final int ACC_PROTECTED = (1 << 2);
    public static final int ACC_ABSTRACT  = (1 << 10);

    // static initialization
    static {
        try {
            objArray = Class.forName("[L" + Object.class.getCanonicalName() + ";");

            Class<?>[] args = new Class<?>[5];
            args[0] = String.class;
            args[1] = Long.TYPE;
            args[2] = Long.TYPE;
            args[3] = Long.TYPE;
            args[4] = objArray;
            mStaticCall = JavaClassBuilder.class.getDeclaredMethod("doStaticCall", args);
            mNormalCall = JavaClassBuilder.class.getDeclaredMethod("doNormalCall", args);
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    static public DynamicType.Builder<?> getClassBuilder(String className, Class<?> parentClass,
            boolean is_abstract, long cptr) throws NoSuchMethodException {
        DynamicType.Builder<?> bb;
        try {
            bb = new ByteBuddy()
                .with(TypeValidation.DISABLED)
                .with(new NamingStrategy.AbstractBase() {
                    @Override
                    public String name(TypeDescription superClass) {
                        return className;
                    }
                })
                .subclass(parentClass, ConstructorStrategy.Default.NO_CONSTRUCTORS);
        } catch (NoClassDefFoundError e) {
            //e.printStackTrace();
            throw new RuntimeException(String.format("qore-jni.jar module not in QORE_CLASSPATH; bytecode " +
                "generation unavailable; cannot perform dynamic imports in Java", e));
        }

        int modifiers = ACC_PUBLIC;
        if (is_abstract) {
            modifiers |= ACC_ABSTRACT;
        }

        bb = bb.modifiers(modifiers);

        // add a static field for storing the class ptr
        bb = (DynamicType.Builder<?>)bb.defineField(CLASS_FIELD, Long.TYPE,
            Modifier.FINAL | Modifier.PUBLIC | Modifier.STATIC)
            .value(cptr);

        // add default constructor
        ArrayList<Type> paramTypes = new ArrayList<Type>();
        paramTypes.add(Long.TYPE);
        paramTypes.add(objArray);

        return (DynamicType.Builder<?>)bb.defineConstructor(Visibility.PUBLIC)
                .withParameters(paramTypes)
                .throwing(Throwable.class)
                .intercept(
                    MethodCall.invoke(parentClass.getConstructor(Long.TYPE, objArray))
                    .onSuper()
                    .withAllArguments()
                );
    }

    // add a constructor
    static public DynamicType.Builder<?> addConstructor(DynamicType.Builder<?> bb, Class<?> parentClass,
            long vptr, int visibility, List<TypeDefinition> paramTypes, boolean varargs) {
        if (paramTypes == null) {
            paramTypes = new ArrayList<TypeDefinition>();
        }

        DynamicType.Builder.MethodDefinition.ExceptionDefinition<?> eb = varargs
                ? bb.defineConstructor(getVisibility(visibility), MethodArguments.VARARGS)
                    .withParameters(paramTypes)
                    .throwing(Throwable.class)
                : bb.defineConstructor(getVisibility(visibility))
                    .withParameters(paramTypes)
                    .throwing(Throwable.class);

        try {
            if (paramTypes.size() == 0) {
                return (DynamicType.Builder<?>)eb.intercept(
                        MethodCall.invoke(parentClass.getConstructor(Long.TYPE, objArray))
                        .onSuper()
                        .withField(CLASS_FIELD)
                        .with((Object)null)
                );
            }

            return (DynamicType.Builder<?>)eb.intercept(
                    MethodCall.invoke(parentClass.getConstructor(Long.TYPE, objArray))
                    .onSuper()
                    .withField(CLASS_FIELD)
                    .withArgumentArray()
            );
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

    // add normal method
    static public DynamicType.Builder<?> addNormalMethod(DynamicType.Builder<?> bb, String methodName, long mptr,
            long vptr, int visibility, TypeDefinition returnType, List<TypeDefinition> paramTypes, boolean isAbstract,
            boolean varargs) {
        if (paramTypes == null) {
            paramTypes = new ArrayList<TypeDefinition>();
        }

        DynamicType.Builder.MethodDefinition.ExceptionDefinition<?> eb =
            varargs
                ? bb.defineMethod(methodName, returnType, getVisibility(visibility), Ownership.MEMBER,
                    MethodArguments.VARARGS)
                    .withParameters(paramTypes)
                    .throwing(Throwable.class)
                : bb.defineMethod(methodName, returnType, getVisibility(visibility), Ownership.MEMBER)
                    .withParameters(paramTypes)
                    .throwing(Throwable.class);

        if (isAbstract) {
            try {
                bb = (DynamicType.Builder<?>)eb.withoutCode();
            } catch (Throwable e) {
                //System.out.println(e.toString());
                throw e;
            }
        } else if (paramTypes.size() == 0) {
            bb = (DynamicType.Builder<?>)eb.intercept(
                    MethodCall.invoke(mNormalCall)
                    .with(methodName)
                    .withField("obj")
                    .with(mptr)
                    .with(vptr)
                    .with((Object)null)
                    .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
                );
        } else {
            bb = (DynamicType.Builder<?>)eb.intercept(
                    MethodCall.invoke(mNormalCall)
                    .with(methodName)
                    .withField("obj")
                    .with(mptr)
                    .with(vptr)
                    .withArgumentArray()
                    .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
            );
        }
        return bb;
    }

    // add static method
    static public DynamicType.Builder<?> addStaticMethod(DynamicType.Builder<?> bb, String methodName, long mptr,
            long vptr, int visibility, TypeDefinition returnType, List<TypeDefinition> paramTypes, boolean varargs) {
        if (paramTypes == null) {
            paramTypes = new ArrayList<TypeDefinition>();
        }

        DynamicType.Builder.MethodDefinition.ExceptionDefinition<?> eb =
            varargs
                ? bb.defineMethod(methodName, returnType, getVisibility(visibility), Ownership.STATIC,
                    MethodArguments.VARARGS)
                    .withParameters(paramTypes)
                    .throwing(Throwable.class)
                : bb.defineMethod(methodName, returnType, getVisibility(visibility), Ownership.STATIC)
                    .withParameters(paramTypes)
                    .throwing(Throwable.class);

        return (DynamicType.Builder<?>)eb.intercept(
                MethodCall.invoke(mStaticCall)
                .with(methodName)
                .withField(CLASS_FIELD)
                .with(mptr)
                .with(vptr)
                .withArgumentArray()
                .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
            );
    }

    @SuppressWarnings("unchecked")
    static public byte[] getByteCodeFromBuilder(DynamicType.Builder<?> bb, QoreURLClassLoader classLoader) {
        DynamicType.Unloaded<?> unloaded = bb.make();
        byte[] byte_code = unloaded.getBytes();
        //System.out.printf("JavaClassBuilder.getClassFromBuilder() %s: got %d bytes (cl: %s)\n",
        //    unloaded.getTypeDescription().getCanonicalName(), byte_code.length, classLoader);
        return byte_code;
    }

    /** makes a static method call
     *
     * @param methodName the name of the method
     * @param args the arguments to the call, if any, can be null
     * @return the result of the call
     * @throws Throwable any exception thrown in Qore
     */
    @RuntimeType
    public static Object doStaticCall(String methodName, long qclsptr, long mptr, long vptr,
            @Argument(0) Object... args) throws Throwable {
        //System.out.println(String.format("JavaClassBuilder::doStaticCall() %s() cptr: %d args: %s", methodName,
        //  qclsptr, Arrays.toString(args)));
        return doStaticCall0(methodName, qclsptr, mptr, vptr, args);
    }

    /** makes a normal method call
     *
     * @param methodName the name of the method
     * @param qobjptr the pointer to the Qore object
     * @param args the arguments to the call, if any, can be null
     * @return the result of the call
     * @throws Throwable any exception thrown in Qore
     */
    @RuntimeType
    public static Object doNormalCall(String methodName, long qobjptr, long mptr, long vptr,
            @Argument(0) Object... args) throws Throwable {
        //System.out.println(String.format("JavaClassBuilder::doNormalCall() %s() ptr: %d args: %s", methodName,
        //  qobjptr, Arrays.toString(args)));
        return doNormalCall0(methodName, qobjptr, mptr, vptr, args);
    }

    /** Returns a TypeDescription object for the given class
     *
     * @param cls the class to return a TypeDescription for
     */
    public static TypeDescription getTypeDescription(Class<?> cls) {
        return new TypeDescription.ForLoadedType(cls);
    }

    /** Returns a TypeDescription for a future type based on the binary name
     *
     * @param future_name The binary name of the type to be created
     */
    public static TypeDescription getTypeDescription(String future_name) {
        return InstrumentedType.Default.of(future_name, null, Modifier.PUBLIC);
    }

    /** Check a class for methods matching a name an TypeDescription list
     *
     */
    public static boolean findBaseClassMethodConflict(Class<?> parentClass, String name, List<TypeDescription> params,
            boolean check_static) {
        for (Method m : parentClass.getMethods()) {
            if (!m.getName().equals(name) || Modifier.isStatic(m.getModifiers()) != check_static) {
                continue;
            }
            // check params
            Type[] mparams = m.getGenericParameterTypes();
            if (mparams == null || mparams.length == 0) {
                if (params == null || params.size() == 0) {
                    return true;
                }
                continue;
            }
            if (mparams.length != params.size()) {
                continue;
            }
            boolean ok = true;
            for (int i = 0; i < mparams.length; ++i) {
                Type mtype = mparams[i];
                TypeDescription ptype = params.get(i);
                if (mtype instanceof Class) {
                    Class cls = (Class)mtype;
                    if (!ptype.getCanonicalName().equals(cls.getCanonicalName())) {
                        ok = false;
                        break;
                    }
                } else if (!ptype.getCanonicalName().equals(mtype.getTypeName())) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                return true;
            }
        }
        return false;
    }

    static private Visibility getVisibility(int visibility) {
        switch (visibility) {
            case ACC_PUBLIC:
                return Visibility.PUBLIC;
            case ACC_PROTECTED:
                return Visibility.PROTECTED;
            default:
                break;
        }

        return Visibility.PRIVATE;
    }

    private static native Object doStaticCall0(String methodName, long qclsptr, long mptr, long vptr, Object... args)
            throws Throwable;
    private static native Object doNormalCall0(String methodName, long qobjptr, long mptr, long vptr, Object... args)
            throws Throwable;
}
