/** Java wrapper for the %Qore SqlStatementOutboundMapper class
 *
 */
package org.qore.lang.tablemapper;

// qorus imports
import org.qore.lang.tablemapper.AbstractSqlStatementOutboundMapper;

// jni module imports
import org.qore.jni.QoreObject;

//! Java wrapper for the @ref TableMapper::SqlStatementOutboundMapper class in Qore
/**
    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.TableMapper.SqlStatementOutboundMapper;</tt>
 */
@Deprecated
public class SqlStatementOutboundMapper extends AbstractSqlStatementOutboundMapper {
    //! creates the object as a wrapper for the Qore object
    public SqlStatementOutboundMapper(QoreObject ds) {
        super(ds);
    }
}
