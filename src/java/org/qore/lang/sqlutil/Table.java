/** Java wrapper for the %Qore Table class
 *
 */
package org.qore.lang.sqlutil;

// jni module imports
import org.qore.jni.QoreJavaApi;
import org.qore.jni.QoreObject;

// Qore imports
import org.qore.lang.AbstractDatasource;
import org.qore.lang.sqlutil.AbstractTable;

//! Java wrapper for the @ref SqlUtil::Table class in %Qore
public class Table extends AbstractTable {
    //! creates a new AbstractTable object from the %Qore Table object
    public Table(QoreObject obj) {
        super(obj);
    }

    //! creates a new AbstractTable object from the given datasource and name
    public Table(AbstractDatasource dsp, String name) throws Throwable {
        super(QoreJavaApi.newObjectSave("SqlUtil::Table", dsp.getQoreObject(), name));
    }
}
