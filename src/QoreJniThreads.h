/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QoreJniThreads.h

  Qore Programming Language JNI Module

  Copyright (C) 2016 Qore Technologies, s.r.o.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _QORE_JNI_QOREJNITHREADS_H

#define _QORE_JNI_QOREJNITHREADS_H

#include <pthread.h>

class QoreJniThreadLock {
public:
   DLLLOCAL QoreJniThreadLock() {
      pthread_mutexattr_t attr;
      pthread_mutexattr_init(&attr);
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(&p_lock, &attr);
      pthread_mutexattr_destroy(&attr);
   }

   DLLLOCAL ~QoreJniThreadLock() {
      pthread_mutex_destroy(&p_lock);
   }

   DLLLOCAL void lock() {
#ifndef NDEBUG
      int rc =
#endif
	 pthread_mutex_lock(&p_lock);
      assert(!rc);
   }

   DLLLOCAL void unlock() {
#ifndef NDEBUG
      int rc =
#endif
	 pthread_mutex_unlock(&p_lock);
      assert(!rc);
   }

   DLLLOCAL int trylock() {
      return pthread_mutex_trylock(&p_lock);
   }

private:
   pthread_mutex_t p_lock;
};

class QoreJniAutoLocker {
private:
   QoreJniAutoLocker(const QoreJniAutoLocker&) = delete;
   QoreJniAutoLocker& operator=(const QoreJniAutoLocker&) = delete;
   void* operator new(size_t) = delete;

protected:
   QoreJniThreadLock* lck;

public:
   DLLLOCAL QoreJniAutoLocker(QoreJniThreadLock* l) : lck(l) {
      lck->lock();
   }

   DLLLOCAL QoreJniAutoLocker(QoreJniThreadLock& l) : lck(&l) {
      lck->lock();
   }

   DLLLOCAL ~QoreJniAutoLocker() {
      lck->unlock();
   }
};

#endif
