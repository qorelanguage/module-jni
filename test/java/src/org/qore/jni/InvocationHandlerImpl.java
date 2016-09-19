package org.qore.jni;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

public class InvocationHandlerImpl implements InvocationHandler {

    private long ptr;
    private int counter;

    InvocationHandlerImpl(long ptr) {
        this.ptr = ptr;
        counter = 1;
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        long p = ref();
        try {
            return invoke0(p, proxy, method, args);
        } finally {
            deref();
        }
    }

    @Override
    protected void finalize() throws Throwable {
        deref();
    }
    
    private synchronized long ref() {
        if (counter == 0) {
            throw new IllegalStateException("Invocation handler has already been destroyed");
        }
        ++counter;
        return ptr;
    }

    private synchronized void deref() {
        if (counter > 0) {
            --counter;
        }
        if (counter == 0) {
            finalize0(ptr);
            ptr = 0;
        }
    }

    private native static void finalize0(long ptr);
    private native Object invoke0(long ptr, Object proxy, Method method, Object[] args) throws Throwable;
}
