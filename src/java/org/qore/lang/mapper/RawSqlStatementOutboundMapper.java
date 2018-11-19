/** Qorus Java SqlStatementOutboundMapper class
 *
 */
package org.qore.lang.mapper;

// qorus imports
import org.qore.lang.tablemapper.AbstractSqlStatementOutboundMapper;

// jni module imports
import org.qore.jni.QoreObject;

//! Java wrapper for the @ref TableMapper::RawSqlStatementOutboundMapper class in Qore
public class RawSqlStatementOutboundMapper extends AbstractSqlStatementOutboundMapper {
    //! creates the object as a wrapper for the Qore object
    public RawSqlStatementOutboundMapper(QoreObject ds) {
        super(ds);
    }
}
