/** Java wrapper for the %Qore AbstractDataProviderBulkOperation class
 *
 */
package org.qore.lang.dataprovider;

// java imports
import java.util.Map;

// jni module imports
import org.qore.lang.AbstractIterator;
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref DataProvider::AbstractDataProviderBulkOperation in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class AbstractDataProviderBulkOperation extends AbstractIterator {
    // static initialization
    static {
        // loads and initializes the Qore library and the jni module (if necessary) and loads the \c DataProvider
        // module
        try {
            QoreJavaApi.initQore();
            QoreJavaApi.callFunction("load_module", "DataProvider");
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates the object as a wrapper for the Qore object
    public AbstractDataProviderBulkOperation(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! Commits data written to the data provider
    /** Has no effect if the data provider does not support transaction management
    */
    public void commit() throws Throwable {
        obj.callMethod("commit");
    }

    //! Rolls back data written to the data provider
    /** Has no effect if the data provider does not support transaction management
    */
    public void rollback() throws Throwable {
        obj.callMethod("rollback");
    }

    //! Queues data in the buffer
    public void queueData(Map<String, Object> record) throws Throwable {
        obj.callMethod("queueData", record);
    }

    //! Queues data in the buffer
    public void queueData(Map<String, Object> records[]) throws Throwable {
        obj.callMethod("queueData", (Object)records);
    }

    //! Flushes any remaining data to the data provider
    /** This method should always be called before committing the transaction (if the data provider supports
        transaction management) or destroying the object
    */
    public void flush() throws Throwable {
        obj.callMethod("flush");
    }

    //! Discards any buffered data
    /** This method should be called before rolling back the transaction (if the data provider supports transaction
        management) or destroying the object if an error occurs
    */
    public void discard() throws Throwable {
        obj.callMethod("discard");
    }
}
