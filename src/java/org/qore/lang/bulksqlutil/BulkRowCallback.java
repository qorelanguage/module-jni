package org.qore.lang.bulksqlutil;

import org.qore.jni.QoreClosureMarker;

import java.util.HashMap;

//! This class is used for row code callbacks with bulk SQL classes
public interface BulkRowCallback extends QoreClosureMarker {
    //! This method is called when data has been sent to the database and all output data is available; must accept a hash argument that represents the data written to the database including any output arguments
    void call(HashMap<String, Object> row);
}
