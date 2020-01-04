/** Java wrapper for the %Qore AbstractDataProviderBulkRecordInterface class
 *
 */
package org.qore.lang.dataprovider;

// java imports
import java.util.HashMap;

// jni module imports
import org.qore.lang.AbstractIterator;
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref DataProvider::AbstractDataProviderBulkRecordInterface in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class AbstractDataProviderBulkRecordInterface extends AbstractIterator {
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
    public AbstractDataProviderBulkRecordInterface(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! Returns the block size
    public int getBlockSize() throws Throwable {
        return (int)obj.callMethod("getBlockSize");
    }

    //! Returns True if there are more records to return
    public boolean valid() throws Throwable {
        return (boolean)obj.callMethod("valid");
    }

    //! Returns a hash of lists according to the block size or @ref nothing if no more data is available
    /** @throw INVALID-ITERATOR the iterator is not pointing at a valid element

        @note This call moves the internal record pointer forward, therefore multiple calls of this methods will
        return different results as long as data is available
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object[]> getValue() throws Throwable {
        return (HashMap<String, Object[]>)obj.callMethod("getValue");
    }
}
