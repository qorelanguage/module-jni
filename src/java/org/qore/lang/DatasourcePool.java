/** Java wrapper for the %Qore AbstractDatasource class
 *
 */
package org.qore.lang;

// qorus imports
import org.qore.lang.AbstractDatasource;

// jni module imports
import org.qore.jni.QoreObject;

//! The main class with common definitions, equivalent to the \c %OMQ namespace in Qorus
public class DatasourcePool extends AbstractDatasource {
    public DatasourcePool(QoreObject ds) {
        super(ds);
    }
}
