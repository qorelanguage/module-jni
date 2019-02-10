/** Java wrapper for the %Qore AbstractDatasource class
 *
 */
package org.qore.lang;

// qorus imports
import org.qore.lang.AbstractDatasource;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! The main class with common definitions, equivalent to the \c %OMQ namespace in Qorus
/** loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class DatasourcePool extends AbstractDatasource {
    // static initialization
    static {
        // initialize the Qore library if necessary
        try {
            QoreJavaApi.initQore();
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    public DatasourcePool(QoreObject ds) {
        super(ds);
    }
}
