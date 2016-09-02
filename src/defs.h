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
/// \brief Contains various definitions.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_DEFS_H_
#define QORE_JNI_DEFS_H_

#include <qore/Qore.h>
#include <jni.h>

namespace jni {

/**
 * \brief Logging level used by the JNI module in calls to printd().
 */
constexpr int LogLevel = 1;

/**
 * \brief Base class for exceptions.
 */
class Exception {

public:
   /**
    * \brief Default virtual destructor.
    */
   virtual ~Exception() = default;

   /**
    * \brief Raises the corresponding Qore exception in the ExceptionSink.
    * \param xsink the exception sink
    */
   virtual void convert(ExceptionSink *xsink) = 0;

   Exception(Exception &&) = default;
   Exception &operator=(Exception &&) = default;

protected:
   /**
    * \brief Default constructor.
    */
   Exception() = default;

private:
   Exception(const Exception &) = delete;
   Exception &operator=(const Exception &) = delete;
};

/**
 * \brief An exception thrown when the current thread cannot be attached to the JVM.
 */
class UnableToAttachException : public Exception {

public:
   /**
    * \brief Constructor.
    * \param err the error code
    */
   UnableToAttachException(jint err) : err(err) {
      printd(LogLevel, "JNI - error attaching thread %d, err: %d\n", gettid(), err);
   }

   void convert(ExceptionSink *xsink) override {
      xsink->raiseException("JNI-ERROR", "Unable to attach thread to the JVM, error %d", err);
   }

private:
   jint err;
};

/**
 * \brief A C++ exception representing a Java exception.
 */
class JavaException : public Exception {

public:
   /**
    * \brief Constructor.
    */
   JavaException() {
   }

   void convert(ExceptionSink *xsink) override;
};

} // namespace jni

#endif // QORE_JNI_DEFS_H_
