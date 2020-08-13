package org.qore.jni;

import java.util.Arrays;

//! The QoreException class is meant to represent a %Qore-language exception
public class QoreException extends RuntimeException {
    //! The %Qore exception error code
    private String err;
    //! The %Qore exception description; if any
    private String desc;
    //! The %Qore exception argument; if any
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

    //! creates the object with the given arguments
    public QoreException(String err) {
        this.err = err;
    }

    //! returns the error string
    String getErr() {
        return err;
    }

    //! returns the description string, if any; may be null
    String getDesc() {
        return desc;
    }

    //! returns the argument, if any; may be null
    Object getArg() {
        return arg;
    }
}
