
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
import net.bytebuddy.description.type.TypeList;
import net.bytebuddy.description.modifier.MethodArguments;
import net.bytebuddy.description.method.MethodDescription.Token;
import net.bytebuddy.description.field.FieldDescription;
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
import net.bytebuddy.implementation.Implementation;
import net.bytebuddy.NamingStrategy;
import net.bytebuddy.matcher.ElementMatchers;

import org.qore.jni.QoreURLClassLoader;
import org.qore.jni.QoreJavaObjectPtr;

/** Helper class for building dynamic Java classes
 */
public class JavaClassBuilder {
    private static Class objArray;
    private static Method mStaticCall;
    private static Method mNormalCall;
    private static Method mFunctionCall;
    private static Method mGetConstantValue;
    private static final String CLASS_FIELD = "$qore_cls_ptr";

    // copied from org.objectweb.asm.Opcodes
    public static final int ACC_PUBLIC    = (1 << 0);
    public static final int ACC_PRIVATE   = (1 << 1);
    public static final int ACC_PROTECTED = (1 << 2);
    public static final int ACC_STATIC    = (1 << 3);
    public static final int ACC_FINAL     = (1 << 4);
    public static final int ACC_ABSTRACT  = (1 << 10);

    //! static initialization
    static {
        try {
            objArray = Class.forName("[L" + Object.class.getCanonicalName() + ";");

            Class<?>[] args = new Class<?>[6];
            args[0] = String.class;
            args[1] = Long.TYPE;
            args[2] = Long.TYPE;
            args[3] = Long.TYPE;
            args[4] = Long.TYPE;
            args[5] = objArray;
            mStaticCall = JavaClassBuilder.class.getDeclaredMethod("doStaticCall", args);

            args = new Class<?>[5];
            args[0] = String.class;
            args[1] = Long.TYPE;
            args[2] = Long.TYPE;
            args[3] = Long.TYPE;
            args[4] = objArray;
            mNormalCall = JavaClassBuilder.class.getDeclaredMethod("doNormalCall", args);

            args = new Class<?>[4];
            args[0] = Long.TYPE;
            args[1] = Long.TYPE;
            args[2] = Long.TYPE;
            args[3] = objArray;
            mFunctionCall = JavaClassBuilder.class.getDeclaredMethod("doFunctionCall", args);

            args = new Class<?>[2];
            args[0] = Long.TYPE;
            args[1] = Long.TYPE;
            mGetConstantValue = JavaClassBuilder.class.getDeclaredMethod("getConstantValue", args);
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! Returns a builder object for a dynamic class mapping Qore functions to static Java methods
    public static DynamicType.Builder<?> getFunctionConstantClassBuilder(String bin_name) throws NoSuchMethodException {
        return new ByteBuddy()
            .with(TypeValidation.DISABLED)
            .with(new NamingStrategy.AbstractBase() {
                @Override
                public String name(TypeDescription superClass) {
                    return bin_name;
                }
            })
            .subclass(Object.class, ConstructorStrategy.Default.NO_CONSTRUCTORS)
            .modifiers(ACC_PUBLIC);
    }

    //! Add a function to a function class
    public static DynamicType.Builder<?> addFunction(DynamicType.Builder<?> bb, String functionName, long pgm,
            long fptr, long vptr, TypeDefinition returnType, List<TypeDefinition> paramTypes, boolean varargs) {
        if (paramTypes == null) {
            paramTypes = new ArrayList<TypeDefinition>();
        }

        DynamicType.Builder.MethodDefinition.ExceptionDefinition<?> eb =
            varargs
                ? bb.defineMethod(functionName, returnType, Visibility.PUBLIC, Ownership.STATIC,
                    MethodArguments.VARARGS)
                    .withParameters(paramTypes)
                    .throwing(Throwable.class)
                : bb.defineMethod(functionName, returnType, Visibility.PUBLIC, Ownership.STATIC)
                    .withParameters(paramTypes)
                    .throwing(Throwable.class);

        return (DynamicType.Builder<?>)eb.intercept(
                MethodCall.invoke(mFunctionCall)
                    .with(pgm)
                    .with(fptr)
                    .with(vptr)
                    .withArgumentArray()
                    .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
            );
    }

    //! Add a field to a class
    public static DynamicType.Builder<?> addStaticField(DynamicType.Builder<?> bb, String fieldName, int modifiers,
            TypeDescription fieldType, long cPtr, ArrayList<StaticEntry> staticList) {
        modifiers |= ACC_FINAL | ACC_STATIC;
        bb = bb.defineField(fieldName, fieldType, modifiers);

        staticList.add(new StaticEntry(fieldName, modifiers, fieldType, cPtr));
        return bb;
    }

    //! Creates the static initializer for a class
    public static DynamicType.Builder<?> createStaticInitializer(DynamicType.Builder<?> bb, String className,
            long pgm, ArrayList<StaticEntry> staticList) {
        Implementation.Composable mc = null;
        for (StaticEntry entry : staticList) {
            Implementation.Composable new_mc = MethodCall.invoke(mGetConstantValue)
                .with(pgm)
                .with(entry.cPtr)
                .setsField(ElementMatchers.is(
                    new FieldDescription.Latent(
                        InstrumentedType.Default.of(className, null, Modifier.PUBLIC
                    ),
                    new FieldDescription.Token(
                        entry.fieldName,
                        entry.modifiers,
                        new TypeDescription.Generic.OfNonGenericType.Latent(entry.fieldType, null))
                    )
                ))
                .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC);
            if (mc == null) {
                mc = new_mc;
            } else {
                mc = mc.andThen(new_mc);
            }
        }

        if (mc == null) {
            return bb;
        }

        return bb.invokable(ElementMatchers.isTypeInitializer())
            .intercept(mc);
    }

    //! Returns a builder object for a dynamic class
    public static DynamicType.Builder<?> getClassBuilder(String className, Class<?> parentClass,
            ArrayList<Type> interfaces, boolean is_abstract, long cptr) throws NoSuchMethodException {
        DynamicType.Builder<?> bb;
        bb = new ByteBuddy()
            .with(TypeValidation.DISABLED)
            .with(new NamingStrategy.AbstractBase() {
                @Override
                public String name(TypeDescription superClass) {
                    return className;
                }
            })
            .subclass(parentClass, ConstructorStrategy.Default.NO_CONSTRUCTORS);

        // add interfaces to class
        if (interfaces != null) {
            for (Type t : interfaces) {
                bb = bb.implement(t);
            }
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

        // add default constructor for already-created Qore objects
        ArrayList<Type> paramTypes = new ArrayList<Type>();
        paramTypes.add(QoreJavaObjectPtr.class);
        bb = (DynamicType.Builder<?>)bb.defineConstructor(Visibility.PUBLIC)
            .withParameters(paramTypes)
            .intercept(
                MethodCall.invoke(parentClass.getConstructor(QoreJavaObjectPtr.class))
                    .onSuper()
                    .withArgument(0)
            );

        // add default constructor for dynamic creation from Qore
        paramTypes = new ArrayList<Type>();
        paramTypes.add(Long.TYPE);
        paramTypes.add(Long.TYPE);
        paramTypes.add(Long.TYPE);
        paramTypes.add(objArray);

        return (DynamicType.Builder<?>)bb.defineConstructor(Visibility.PUBLIC)
            .withParameters(paramTypes)
            .throwing(Throwable.class)
            .intercept(
                MethodCall.invoke(parentClass.getConstructor(Long.TYPE, Long.TYPE, Long.TYPE, objArray))
                    .onSuper()
                    .withAllArguments()
            );
    }

    //! add a constructor
    public static DynamicType.Builder<?> addConstructor(DynamicType.Builder<?> bb, Class<?> parentClass,
            long mptr, long vptr, int visibility, List<TypeDefinition> paramTypes, boolean varargs) {
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
                        MethodCall.invoke(parentClass.getConstructor(Long.TYPE, Long.TYPE, Long.TYPE, objArray))
                            .onSuper()
                            .withField(CLASS_FIELD)
                            .with(mptr)
                            .with(vptr)
                            .with((Object)null)
                );
            }

            return (DynamicType.Builder<?>)eb.intercept(
                    MethodCall.invoke(parentClass.getConstructor(Long.TYPE, Long.TYPE, Long.TYPE, objArray))
                        .onSuper()
                        .withField(CLASS_FIELD)
                        .with(mptr)
                        .with(vptr)
                        .withArgumentArray()
            );
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

    //! add normal method
    public static DynamicType.Builder<?> addNormalMethod(DynamicType.Builder<?> bb, String methodName, long mptr,
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

    //! add static method
    public static DynamicType.Builder<?> addStaticMethod(DynamicType.Builder<?> bb, String methodName, long pgm, long mptr,
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
                .with(pgm)
                .with(mptr)
                .with(vptr)
                .withArgumentArray()
                .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
            );
    }

    @SuppressWarnings("unchecked")
    public static byte[] getByteCodeFromBuilder(DynamicType.Builder<?> bb, QoreURLClassLoader classLoader) {
        return bb.make().getBytes();
    }

    /** makes a static method call
     *
     * @param methodName the name of the method
     * @param qclsptr the class pointer
     * @param mptr the method pointer
     * @param vptr the variant pointer
     * @param args the arguments to the call, if any, can be null
     * @return the result of the call
     * @throws Throwable any exception thrown in Qore
     */
    @RuntimeType
    public static Object doStaticCall(String methodName, long qclsptr, long pgm, long mptr, long vptr,
            @Argument(0) Object... args) throws Throwable {
        //System.out.println(String.format("JavaClassBuilder::doStaticCall() %s() cptr: %d args: %s", methodName,
        //  qclsptr, Arrays.toString(args)));
        return doStaticCall0(methodName, qclsptr, pgm, mptr, vptr, args);
    }

    /** makes a normal method call
     *
     * @param methodName the name of the method
     * @param qobjptr the pointer to the Qore object
     * @param mptr the pointer to the Qore method object
     * @param vptr the pointer to the method variant object
     * @param args the arguments to the call, if any, can be null
     *
     * @return the result of the call
     *
     * @throws Throwable any exception thrown in Qore
     */
    @RuntimeType
    public static Object doNormalCall(String methodName, long qobjptr, long mptr, long vptr,
            @Argument(0) Object... args) throws Throwable {
        //System.out.println(String.format("JavaClassBuilder::doNormalCall() %s() ptr: %d args: %s", methodName,
        //  qobjptr, Arrays.toString(args)));
        return doNormalCall0(methodName, qobjptr, mptr, vptr, args);
    }

    /** makes a function call
     *
     * @param fptr the pointer to the Qore function object
     * @param vptr the pointer to the method variant object
     * @param args the arguments to the call, if any, can be null
     *
     * @return the result of the call
     *
     * @throws Throwable any exception thrown in Qore
     */
    @RuntimeType
    public static Object doFunctionCall(long pgm, long fptr, long vptr, @Argument(0) Object... args)
            throws Throwable {
        //System.out.println(String.format("JavaClassBuilder::doFunctionCall() %s() args: %s", methodName,
        //  Arrays.toString(args)));
        return doFunctionCall0(pgm, fptr, vptr, args);
    }

    /** retrieves the value of a constant from the given Qore program
     *
     * @param pgm the pointer to the Qore program object
     * @param cPtr the pointer the constant entry
     *
     * @return the value of the given constant
     */
    @RuntimeType
    public static Object getConstantValue(long pgm, long cPtr) throws Throwable {
        return getConstantValue0(pgm, cPtr);
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

    private static native Object doStaticCall0(String methodName, long qclsptr, long pgm, long mptr, long vptr,
            Object... args) throws Throwable;
    private static native Object doNormalCall0(String methodName, long qobjptr, long mptr, long vptr, Object... args)
            throws Throwable;
    private static native Object doFunctionCall0(long pgm, long fptr, long vptr, @Argument(0) Object... args)
            throws Throwable;
    private static native Object getConstantValue0(long pgm, long cPtr) throws Throwable;
}

class StaticEntry {
    public String fieldName;
    public int modifiers;
    public TypeDescription fieldType;
    public long cPtr;

    StaticEntry(String fieldName, int modifiers, TypeDescription fieldType, long cPtr) {
        this.fieldName = fieldName;
        this.modifiers = modifiers;
        this.fieldType = fieldType;
        this.cPtr = cPtr;
    }
}