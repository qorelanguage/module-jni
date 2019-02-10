/** Java wrapper for the %Qore Mapper class
 *
 */
package org.qore.lang.mapper;

// java imports
import java.util.HashMap;
import java.util.Map;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref Mapper::Mapper class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class Mapper extends QoreObjectWrapper {
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
    public Mapper(QoreObject obj) {
        super(obj);
    }

    //! get current @ref mapper_runtime_handling "runtime option" value for a key
    /**
        @param key the runtime option key
        @returns a runtime value if the key exists in the current @ref mapper_runtime_handling "runtime option hash" and is set

        @see @ref mapper_runtime_handling

        @since %Mapper 1.1
     */
    public Object getRuntime(String key) throws Throwable {
        return obj.callMethod("getRuntime", key);
    }

    //! returns a descriptive name of the given field if possible, otherwise returns the field name itself
    public String getFieldName(String fname) throws Throwable {
        return (String)obj.callMethod("getFieldName", fname);
    }

    //! returns a list of valid field keys for this class (can be overridden in subclasses)
    /** @return a list of valid field keys for this class (can be overridden in subclasses)
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> validKeys() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("validKeys");
    }

    //! returns a list of valid field types for this class (can be overridden in subclasses)
    /** @return a list of valid types for this class (can be overridden in subclasses)
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> validTypes() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("validTypes");
    }

    //! returns a list of valid constructor options for this class (can be overridden in subclasses)
    /** @return a list of valid constructor options for this class (can be overridden in subclasses)
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> optionKeys() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("optionKeys");
    }

    //! returns the value of the \c "input" option
    /** @note null is returned if no input record is set
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getInputRecord() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getInputRecord");
    }

    //! returns the value of the \c "output" option
    /** @note null is returned if no output record is set
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getOutputRecord() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getOutputRecord");
    }

    //! maps all input records and returns the mapped data as a list of output records
    /** this method applies the @ref mapData() method to all input records and returns the resulting list
        @param recs the list of input records

        @return the mapped data as a list of output records

        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] mapAll(Map<String, Object>[] recs) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("mapAll", (Object)recs);
    }

    //! maps all input records and returns the mapped data as a list of output records
    /** this method applies the @ref mapData() method to all input records and returns the resulting list
        @param recs a Map<String, Object> of lists of input records

        @return the mapped data as a list of output records

        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] mapAll(Map<String, Object> recs) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("mapAll", recs);
    }

    //! processes the input record and returns a Map<String, Object> of the mapped values where the keys in the Map<String, Object> returned are the target field names; the order of the fields in the Map<String, Object> returned is the same order as the keys in the map hash.
    /** @param rec the record to translate

        @return a Map<String, Object> of field values in the target format based on the input data and processed according to the logic in the map hash

        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data

        @note
        - each time this method is executed successfully, the record count is updated (see @ref getCount() and @ref resetCount())
        - uses @ref Mapper::Mapper::mapDataIntern() to map the data, then @ref Mapper::Mapper::logOutput() is called for each output row
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> mapData(Map<String, Object> rec) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("mapData", rec);
    }
}

