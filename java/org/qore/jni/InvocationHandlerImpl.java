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
        cleanup();
    }

    private synchronized long ref() {
        if (counter == 0) {
            throw new IllegalStateException("Invocation handler has already been destroyed");
        }
        ++counter;
        return ptr;
    }

    private synchronized void deref() {
        --counter;
        if (counter == 1) {
            notify();
        }
    }

    private synchronized void destroy() {
        while (true) {
            if (counter == 0) {
                return;
            }
            if (counter == 1) {
                cleanup();
                return;
            }
            try {
                wait();
            } catch (InterruptedException e) {
                // ignored
            }
        }
    }

    private void cleanup() {
        long p = ptr;
        counter = 0;
        ptr = 0;
        finalize0(p);
    }

    private native static void finalize0(long ptr);
    private native Object invoke0(long ptr, Object proxy, Method method, Object[] args) throws Throwable;
}
