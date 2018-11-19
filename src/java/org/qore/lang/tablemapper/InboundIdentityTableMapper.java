/** Java wrapper for the %Qore InboundIdentityTableMapper class
 *
 */
package org.qore.lang.tablemapper;

// qorus imports
import org.qore.lang.mapper.Mapper;

// jni module imports
import org.qore.jni.QoreObject;

//! Java wrapper for the @ref TableMapper::InboundIdentityTableMapper class in Qore
public class InboundIdentityTableMapper extends InboundTableMapper {
    //! creates the object as a wrapper for the Qore object
    public InboundIdentityTableMapper(QoreObject obj) {
        super(obj);
    }
}
