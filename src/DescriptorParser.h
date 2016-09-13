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
/// \brief Defines the DescriptorParser class.
///
//------------------------------------------------------------------------------
#ifndef QORE_JNI_DESCRIPTORPARSER_H_
#define QORE_JNI_DESCRIPTORPARSER_H_

#include <cassert>
#include <string>

namespace jni {

class DescriptorParser {

public:
   DescriptorParser(const std::string &descriptor) : descriptor(descriptor), pos(0) {
      //we assume that the descriptor is well-formed since it was successfully used to get the jfieldID/jmethodID
   }

   char getType() {
      return descriptor[pos++];
   }

   std::string getClassName() {
      std::string::size_type begin = pos;
      pos = descriptor.find(';', begin);
      assert(pos != std::string::npos);
      ++pos;
      return descriptor.substr(begin, pos - begin - 1);
   }

   std::string getArrayDescriptor() {
      assert(pos > 0 && descriptor[pos - 1] == '[');
      std::string::size_type begin = pos - 1;

      while (descriptor[pos] == '[') {
         ++pos;
      }
      if (descriptor[pos] == 'L') {
         pos = descriptor.find(';', pos);
         assert(pos != std::string::npos);
      }
      ++pos;
      return descriptor.substr(begin, pos - begin);
   }

protected:
   const std::string &descriptor;
   std::string::size_type pos;
};

} // namespace jni

#endif // QORE_JNI_DESCRIPTORPARSER_H_
