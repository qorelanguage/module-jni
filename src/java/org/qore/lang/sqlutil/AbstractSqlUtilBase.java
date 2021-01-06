/** Java wrapper for the %Qore AbstractTable class
 *
 */
package org.qore.lang.sqlutil;

// java imports
import java.util.Map;
import java.util.Map;
import java.util.Collections;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;
import org.qore.jni.QoreJavaApi;

// Qore Java imports
import org.qore.lang.AbstractDatasource;

//! Java wrapper for the @ref SqlUtil::AbstractSqlUtilBase class in %Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.SqlUtil.AbstractSqlUtilBase;</tt>
*/
@Deprecated
public class AbstractSqlUtilBase extends QoreObjectWrapper {
    // static initialization
    static {
        // loads and initializes the Qore library and the jni module (if necessary) and loads the \c SqlUtil module
        try {
            QoreJavaApi.initQore();
            QoreJavaApi.callFunction("load_module", "SqlUtil");
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates the object from a weak reference to the Qore object
    public AbstractSqlUtilBase(QoreObject obj) {
        super(obj);
    }

    //! gets the underlying AbstractDatasource
    public AbstractDatasource getDatasource() throws Throwable {
        return new AbstractDatasource((QoreObject)obj.callMethod("getDatasource"));
    }

    //! returns the database driver name
    public String getDriverName() throws Throwable {
        return (String)obj.callMethod("getDriverName");
    }

    //! returns a descriptive string for the datasource
    public String getDatasourceDesc() throws Throwable {
        return (String)obj.callMethod("getDatasourceDesc");
    }
}
