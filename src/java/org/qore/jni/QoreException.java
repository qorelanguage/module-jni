package org.qore.jni;

import java.util.Arrays;

public class QoreException extends RuntimeException {
    private String err;
    private String desc;
    private Object arg;

    private static final long serialVersionUID = 1L;

    //! creates the object with the given arguments
    public QoreException(String err, String desc, Object arg) {
        this.err = err;
        this.desc = desc;
        this.arg = arg;
    }

    //! creates the object with the given arguments
    public QoreException(String err, String desc) {
        this.err = err;
        this.desc = desc;
    }

    //! returns the error string
    String getErr() {
        return err;
    }

    //! returns the description string
    String getDesc() {
        return desc;
    }

    //! returns the argument, if any; may be null
    Object getArg() {
        return arg;
    }
}
