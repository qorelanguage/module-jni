//--------------------------------------------------------------------*- C++ -*-
//
//  Qore Programming Language
//
//  Copyright (C) 2015 Qore Technologies
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//------------------------------------------------------------------------------
///
/// \file
/// \brief Defines the Env class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_ENV_H_
#define QORE_JNI_ENV_H_

#include "Jvm.h"
#include "LocalReference.h"

namespace jni {

/**
 * \brief Provides access to JNI functions.
 *
 * Wraps JNI references to RAII objects and uses C++ exceptions for error reporting.
 */
class Env {

public:
   /**
    * \brief Default constructor. Attaches current thread to the JVM.
    * \throws UnableToAttachException if the thread cannot be attached to the JVM
    */
   Env() : env(Jvm::attachAndGetEnv()) {
   }

   /**
    * \brief Returns the major version number in the higher 16 bits and the minor version number in the lower 16 bits.
    * \return the major version number in the higher 16 bits and the minor version number in the lower 16 bits
    */
   jint getVersion() {
      return env->GetVersion();
   }

   /**
    * \brief Finds a class with given name.
    * \param name fully-qualified class name or an array type signature
    * \return a local reference to the class object
    * \throws JavaException if the class cannot be found
    */
   LocalReference<jclass> findClass(const char *name) {
      jclass c = env->FindClass(name);
      if (c == nullptr) {
         throw JavaException();
      }
      return c;
   }

   /**
    * \brief Finds a field in a class.
    * \param clazz the class
    * \param name the name of the field
    * \param descriptor the field descriptor
    * \return field id
    * \throws JavaException if the field cannot be found
    */
   jfieldID getField(jclass clazz, const char *name, const char *descriptor) {
      jfieldID id = env->GetFieldID(clazz, name, descriptor);
      if (id == nullptr) {
         throw JavaException();
      }
      return id;
   }

   /**
    * \brief Finds a static field in a class.
    * \param clazz the class
    * \param name the name of the field
    * \param descriptor the field descriptor
    * \return field id
    * \throws JavaException if the field cannot be found
    */
   jfieldID getStaticField(jclass clazz, const char *name, const char *descriptor) {
      jfieldID id = env->GetStaticFieldID(clazz, name, descriptor);
      if (id == nullptr) {
         throw JavaException();
      }
      return id;
   }

   /**
    * \brief Finds a method in a class.
    * \param clazz the class
    * \param name the name of the method
    * \param descriptor the method signature
    * \return method id
    * \throws JavaException if the method cannot be found
    */
   jmethodID getMethod(jclass clazz, const char *name, const char *descriptor) {
      jmethodID id = env->GetMethodID(clazz, name, descriptor);
      if (id == nullptr) {
         throw JavaException();
      }
      return id;
   }

   /**
    * \brief Finds a static method in a class.
    * \param clazz the class
    * \param name the name of the method
    * \param descriptor the method signature
    * \return method id
    * \throws JavaException if the method cannot be found
    */
   jmethodID getStaticMethod(jclass clazz, const char *name, const char *descriptor) {
      jmethodID id = env->GetStaticMethodID(clazz, name, descriptor);
      if (id == nullptr) {
         throw JavaException();
      }
      return id;
   }

   /**
    * \brief Gets the value of a boolean field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jboolean getBooleanField(jobject object, jfieldID id) {
      return env->GetBooleanField(object, id);
   }

   /**
    * \brief Gets the value of a byte field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jbyte getByteField(jobject object, jfieldID id) {
      return env->GetByteField(object, id);
   }

   /**
    * \brief Gets the value of a char field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jchar getCharField(jobject object, jfieldID id) {
      return env->GetCharField(object, id);
   }

   /**
    * \brief Gets the value of a double field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jdouble getDoubleField(jobject object, jfieldID id) {
      return env->GetDoubleField(object, id);
   }

   /**
    * \brief Gets the value of a float field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jfloat getFloatField(jobject object, jfieldID id) {
      return env->GetFloatField(object, id);
   }

   /**
    * \brief Gets the value of an int field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jint getIntField(jobject object, jfieldID id) {
      return env->GetIntField(object, id);
   }

   /**
    * \brief Gets the value of a long field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jlong getLongField(jobject object, jfieldID id) {
      return env->GetLongField(object, id);
   }

   /**
    * \brief Gets the value of an Object field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   LocalReference<jobject> getObjectField(jobject object, jfieldID id) {
      return env->GetObjectField(object, id);
   }

   /**
    * \brief Gets the value of a short field.
    * \param object the instance
    * \param id the field id
    * \return the field value
    */
   jshort getShortField(jobject object, jfieldID id) {
      return env->GetShortField(object, id);
   }

   /**
    * \brief Gets the value of a static boolean field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jboolean getStaticBooleanField(jclass clazz, jfieldID id) {
      return env->GetStaticBooleanField(clazz, id);
   }

   /**
    * \brief Gets the value of a static byte field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jbyte getStaticByteField(jclass clazz, jfieldID id) {
      return env->GetStaticByteField(clazz, id);
   }

   /**
    * \brief Gets the value of a static char field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jchar getStaticCharField(jclass clazz, jfieldID id) {
      return env->GetStaticCharField(clazz, id);
   }

   /**
    * \brief Gets the value of a static double field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jdouble getStaticDoubleField(jclass clazz, jfieldID id) {
      return env->GetStaticDoubleField(clazz, id);
   }

   /**
    * \brief Gets the value of a static float field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jfloat getStaticFloatField(jclass clazz, jfieldID id) {
      return env->GetStaticFloatField(clazz, id);
   }

   /**
    * \brief Gets the value of a static int field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jint getStaticIntField(jclass clazz, jfieldID id) {
      return env->GetStaticIntField(clazz, id);
   }

   /**
    * \brief Gets the value of a static long field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jlong getStaticLongField(jclass clazz, jfieldID id) {
      return env->GetStaticLongField(clazz, id);
   }

   /**
    * \brief Gets the value of a static Object field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   LocalReference<jobject> getStaticObjectField(jclass clazz, jfieldID id) {
      return env->GetStaticObjectField(clazz, id);
   }

   /**
    * \brief Gets the value of a static short field.
    * \param clazz the class of the field
    * \param id the field id
    * \return the field value
    */
   jshort getStaticShortField(jclass clazz, jfieldID id) {
      return env->GetStaticShortField(clazz, id);
   }

   /**
    * \brief Sets the value of a boolean field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setBooleanField(jobject object, jfieldID id, jboolean value) {
      env->SetBooleanField(object, id, value);
   }

   /**
    * \brief Sets the value of a byte field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setByteField(jobject object, jfieldID id, jbyte value) {
      env->SetByteField(object, id, value);
   }

   /**
    * \brief Sets the value of a char field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setCharField(jobject object, jfieldID id, jchar value) {
      env->SetCharField(object, id, value);
   }

   /**
    * \brief Sets the value of a double field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setDoubleField(jobject object, jfieldID id, jdouble value) {
      env->SetDoubleField(object, id, value);
   }

   /**
    * \brief Sets the value of a float field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setFloatField(jobject object, jfieldID id, jfloat value) {
      env->SetFloatField(object, id, value);
   }

   /**
    * \brief Sets the value of a int field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setIntField(jobject object, jfieldID id, jint value) {
      env->SetIntField(object, id, value);
   }

   /**
    * \brief Sets the value of a long field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setLongField(jobject object, jfieldID id, jlong value) {
      env->SetLongField(object, id, value);
   }

   /**
    * \brief Sets the value of a Object field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setObjectField(jobject object, jfieldID id, jobject value) {
      env->SetObjectField(object, id, value);
   }

   /**
    * \brief Sets the value of a short field.
    * \param object the instance
    * \param id the field id
    * \param value the field value
    */
   void setShortField(jobject object, jfieldID id, jshort value) {
      env->SetShortField(object, id, value);
   }

   /**
    * \brief Sets the value of a static boolean field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticBooleanField(jclass clazz, jfieldID id, jboolean value) {
      env->SetStaticBooleanField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static byte field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticByteField(jclass clazz, jfieldID id, jbyte value) {
      env->SetStaticByteField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static char field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticCharField(jclass clazz, jfieldID id, jchar value) {
      env->SetStaticCharField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static double field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticDoubleField(jclass clazz, jfieldID id, jdouble value) {
      env->SetStaticDoubleField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static float field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticFloatField(jclass clazz, jfieldID id, jfloat value) {
      env->SetStaticFloatField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static int field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticIntField(jclass clazz, jfieldID id, jint value) {
      env->SetStaticIntField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static long field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticLongField(jclass clazz, jfieldID id, jlong value) {
      env->SetStaticLongField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static Object field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticObjectField(jclass clazz, jfieldID id, jobject value) {
      env->SetStaticObjectField(clazz, id, value);
   }

   /**
    * \brief Sets the value of a static short field.
    * \param clazz the class of the field
    * \param id the field id
    * \param value the field value
    */
   void setStaticShortField(jclass clazz, jfieldID id, jshort value) {
      env->SetStaticShortField(clazz, id, value);
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jboolean callStaticBooleanMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jboolean ret = env->CallStaticBooleanMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jbyte callStaticByteMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jbyte ret = env->CallStaticByteMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jchar callStaticCharMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jchar ret = env->CallStaticCharMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jdouble callStaticDoubleMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jdouble ret = env->CallStaticDoubleMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jfloat callStaticFloatMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jfloat ret = env->CallStaticFloatMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jint callStaticIntMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jint ret = env->CallStaticIntMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jlong callStaticLongMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jlong ret = env->CallStaticLongMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   LocalReference<jobject> callStaticObjectMethod(jclass clazz, jmethodID id, const jvalue *args) {
      LocalReference<jobject> ret = env->CallStaticObjectMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \return the return value
    * \throws JavaException if the method throws an exception
    */
   jshort callStaticShortMethod(jclass clazz, jmethodID id, const jvalue *args) {
      jshort ret = env->CallStaticShortMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
      return ret;
   }

   /**
    * \brief Invokes a static method.
    * \param clazz the class of the method
    * \param id the method id
    * \param args the arguments
    * \throws JavaException if the method throws an exception
    */
   void callStaticVoidMethod(jclass clazz, jmethodID id, const jvalue *args) {
      env->CallStaticVoidMethodA(clazz, id, args);
      if (env->ExceptionCheck()) {
         throw JavaException();
      }
   }

   /**
    * \brief Tests whether an object is an instance of a class.
    * \param obj the object
    * \param clazz the class
    * \return true if obj can be cast to clazz
    */
   bool isInstanceOf(jobject obj, jclass clazz) {
      return env->IsInstanceOf(obj, clazz) == JNI_TRUE;
   }

private:
   JNIEnv *env;
};

} // namespace jni

#endif // QORE_JNI_ENV_H_
