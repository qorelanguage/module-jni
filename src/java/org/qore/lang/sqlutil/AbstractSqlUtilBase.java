/** Java wrapper for the %Qore AbstractTable class
 *
 */
package org.qore.lang.sqlutil;

// java imports
import java.util.Map;
import java.util.HashMap;
import java.util.Collections;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;

//! Java wrapper for the @ref SqlUtil::AbstractSqlUtilBase class in %Qore
public class AbstractSqlUtilBase extends QoreObjectWrapper {
    //! creates the object from a weak reference to the Qore object
    public AbstractSqlUtilBase(QoreObject obj) {
        super(obj);
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
