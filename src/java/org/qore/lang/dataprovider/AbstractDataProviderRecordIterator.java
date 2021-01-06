/** Java wrapper for the %Qore AbstractDataProviderRecordIterator class
 *
 */
package org.qore.lang.dataprovider;

// java imports
import java.util.HashMap;

// jni module imports
import org.qore.lang.AbstractIterator;
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref DataProvider::AbstractDataProviderRecordIterator class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.DataProvider.AbstractDataProviderRecordIterator;</tt>
*/
@Deprecated
public class AbstractDataProviderRecordIterator extends AbstractIterator {
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
    public AbstractDataProviderRecordIterator(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! returns a single record if the iterator is valid
    /** @throw INVALID-ITERATOR the iterator is not pointing at a valid element
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getValue() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getValue");
    }
}
