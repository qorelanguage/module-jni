/** Qorus Java SqlStatementOutboundMapper class
 *
 */
package org.qore.lang.tablemapper;

// qorus imports
import org.qore.lang.tablemapper.AbstractSqlStatementOutboundMapper;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref TableMapper::RawSqlStatementOutboundMapper class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.TableMapper.RawSqlStatementOutboundMapper;</tt>
*/
@Deprecated
public class RawSqlStatementOutboundMapper extends AbstractSqlStatementOutboundMapper {
    // static initialization
    static {
        // loads and initializes the Qore library and the jni module (if necessary) and loads the \c TableMapper module
        try {
            QoreJavaApi.initQore();
            QoreJavaApi.callFunction("load_module", "TableMapper");
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates the object as a wrapper for the Qore object
    public RawSqlStatementOutboundMapper(QoreObject ds) {
        super(ds);
    }
}
