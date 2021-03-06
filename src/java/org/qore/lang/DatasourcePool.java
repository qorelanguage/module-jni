/** Java wrapper for the %Qore AbstractDatasource class
 *
 */
package org.qore.lang;

// qorus imports
import org.qore.lang.AbstractDatasource;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref Qore::SQL::DatasourcePool class in %Qore
/** loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qore.Qore.SQL.DatasourcePool;</tt>
 */
@Deprecated
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

    //! creates the object from the %Qore object
    public DatasourcePool(QoreObject ds) {
        super(ds);
    }

    //! creates the object from the config string (ex: \c "pgsql:user/pass@db")
    public DatasourcePool(String config) throws Throwable {
        super(QoreJavaApi.newObjectSave("DatasourcePool", config));
    }
}
