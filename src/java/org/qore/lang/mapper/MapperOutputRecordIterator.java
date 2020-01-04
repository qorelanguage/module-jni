/** Java wrapper for the %Qore MapperOutputRecordIterator class
 *
 */
package org.qore.lang.mapper;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;
import org.qore.lang.dataprovider.AbstractDataProviderRecordIterator;

//! Java wrapper for the @ref Mapper::MapperOutputRecordIterator class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class MapperOutputRecordIterator extends AbstractDataProviderRecordIterator {
    // static initialization
    static {
        // loads and initializes the Qore library and the jni module (if necessary) and loads the \c Mapper module
        try {
            QoreJavaApi.initQore();
            QoreJavaApi.callFunction("load_module", "Mapper");
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates the object as a wrapper for the Qore object
    public MapperOutputRecordIterator(QoreObject obj) throws Throwable {
        super(obj);
    }
}
