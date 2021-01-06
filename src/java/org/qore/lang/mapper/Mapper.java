/** Java wrapper for the %Qore Mapper class
 *
 */
package org.qore.lang.mapper;

// java imports
import java.util.HashMap;
import java.util.Map;

// jni module imports
import org.qore.jni.Hash;
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;
import org.qore.jni.QoreJavaApi;

/**
    Java wrapper for the @see Mapper::Mapper class in Qore

    @note Loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.Mapper.Mapper;</tt>
*/
@Deprecated
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

    /**
     * creates the object as a wrapper for the Qore object
     * @param obj the Qore object
     */
    public Mapper(QoreObject obj) {
        super(obj);
    }

    /**
     * get current @see mapper_runtime_handling "runtime option" value for a key
     *  @param key the runtime option key
     *  @return a runtime value if the key exists in the current @see mapper_runtime_handling "runtime option hash" and is set
     *  @see mapper_runtime_handling
     *  @since %Mapper 1.1
     */
    public Object getRuntime(String key) throws Throwable {
        return obj.callMethod("getRuntime", key);
    }

    /**
     * @param fname the field
     * @return a descriptive name of the given field if possible, otherwise returns the field name itself
     */
    public String getFieldName(String fname) throws Throwable {
        return (String)obj.callMethod("getFieldName", fname);
    }

    /**
     * @return a list of valid field keys for this class (can be overridden in subclasses)
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> validKeys() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("validKeys");
    }

    /**
     * @return a list of valid types for this class (can be overridden in subclasses)
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> validTypes() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("validTypes");
    }

    /**
     * @return a list of valid constructor options for this class (can be overridden in subclasses)
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> optionKeys() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("optionKeys");
    }

    /**
     * @note null is returned if no input record is set
     * @return the value of the \c "input" option
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getInputRecord() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getInputRecord");
    }

    /**
     * @note null is returned if no output record is set
     * @return the value of the \c "output" option
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getOutputRecord() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getOutputRecord");
    }

    /**
     * @return an output record iterator that produces mapped data from the input data provider
     * @throws MAPPER-INPUT-PROVIDER-ERROR if no \a input_provider option was provided in the constructor or the input
     * provider uses the request/response API
     */
    public MapperOutputRecordIterator getOutputIterator() throws Throwable {
        return new MapperOutputRecordIterator((QoreObject)obj.callMethodSave("getOutputIterator"));
    }

    /**
     * Commits data written to the output data provider if the output data provider supports transaction management
     * Has no effect if the output data provider does not support transaction management
     * @throws MAPPER-OUTPUT-PROVIDER-ERROR no output provider available in this mapper
     */
    public void commit() throws Throwable {
        obj.callMethod("commit");
    }

    /**
     * Rolls back data written to the output data provider
     * Has no effect if the output data provider does not support transaction management
     * @throws MAPPER-OUTPUT-PROVIDER-ERROR no output provider available in this mapper
     */
    public void rollback() throws Throwable {
        obj.callMethod("rollback");
    }

    /**
     * Flushes any remaining data to the data provider
     * This method should always be called for successful bulk output operations with an output provider
     * @throws MAPPER-OUTPUT-BULK-ERROR no output bulk operation is in progress
     */
    public void flushOutput() throws Throwable {
        obj.callMethod("flushOutput");
    }

    /**
     * Discards any buffered data
     * This method should always be called if an error occurs in a bulk output operation
     * @throws MAPPER-OUTPUT-BULK-ERROR no output bulk operation is in progress
     */
    public void discardOutput() throws Throwable {
        obj.callMethod("discardOutput");
    }

    /**
     * Runs the input and output mappers with data providers on each end autonomously
     * @throws MAPPER-INPUT-PROVIDER-ERROR no input provider available in this mapper
     * @throws MAPPER-OUTPUT-PROVIDER-ERROR no output provider available in this mapper
     */
    public void runAutonomous() throws Throwable {
        obj.callMethod("runAutonomous");
    }

    /**
     * maps all input records and returns the mapped data as a list of output records
     * this method applies the @see mapData() method to all input records and returns the resulting list
     * @param recs the list of input records
     * @return the mapped data as a list of output records
     * @throws MISSING-INPUT a field marked mandatory is missing
     * @throws STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
     * @throws INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] mapAll(Map<String, Object>[] recs) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("mapAll", (Object)recs);
    }

    /**
     * maps all input records and returns the mapped data as a list of output records
     * this method applies the @see mapData() method to all input records and returns the resulting list
     * @param recs a Map<String, Object> of lists of input records
     * @return the mapped data as a list of output records
     * @throws MISSING-INPUT a field marked mandatory is missing
     * @throws STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
     * @throws INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] mapAll(Map<String, Object> recs) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("mapAll", recs);
    }

    /**
     * processes the input record and returns a Map<String, Object> of the mapped values where the keys in the Map<String, Object> returned are the target field names; the order of the fields in the Map<String, Object> returned is the same order as the keys in the map hash.
     * @param rec the record to translate
     * @return a Map<String, Object> of field values in the target format based on the input data and processed according to the logic in the map hash
     * @throws MISSING-INPUT a field marked mandatory is missing
     * @throws STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
     * @throws INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
     * @note
     * - each time this method is executed successfully, the record count is updated (see @see getCount() and @see resetCount())
     * - uses @see Mapper::Mapper::mapDataIntern() to map the data, then @see Mapper::Mapper::logOutput() is called for each output row
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> mapData(Map<String, Object> rec) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("mapData", rec);
    }

    /**
     * maps all input record(s) automatically and returns the mapped data
     * this method applies the @see mapAll() method if the input records are given as list and the @see mapBulk() method if the input is a Map<String, Object> of lists.
     * For a simple Map<String, Object> (one record) it calls the @see mapData() method.
     * @param recs input records (one record or more), can be represented by list or Map<String, Object>
     * @return the mapped data as of output records (type depends on the given records' type)
     * @throws MISSING-INPUT a field marked mandatory is missing
     * @throws STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
     * @throws INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
     * @throws MAPPER-INPUT-TYPE-ERROR records input type is not acceptable
     * @since %Mapper 1.6
     */
    public Object mapAuto(Object recs) throws Throwable {
        return obj.callMethod("mapAuto", recs);
    }

    /**
     * maps a set of records in Map<String, Object> of lists format; returns mapped data in a Hash of lists format
     * @par Example:
     * @code{.java}
     * Hash output = mapper.mapBulk(h);
     * @endcode
     * @param rec the input record or record set in case a Map<String, Object> of lists is passed
     * @param crec an optional Map<String, Object> of data to be added to each input row before mapping
     * @return returns a Hash of lists of mapped data
     * @note
     * - using a Map<String, Object> of lists in \a rec; note that this provides very high performance when used with data
     * provider output that support bulk write operations
     * - in case a Map<String, Object> of empty lists is passed, null is returned
     * - 'crec' does not affect the number of output lines; in particular, if 'rec' is a batch with \c N rows of
     * a column \c C and 'crec = {"C": "mystring"}' then the output will be as if there was 'N' rows with
     * \c C = "mystring" on the input.
     * @see
     * - flushOutput()
     * - discardOutput()
     * @throws MISSING-INPUT a field marked mandatory is missing
     * @throws STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
     * @throws INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
     */
    public Hash mapBulk(Map<String, Object> rec, Map<String, Object> crec) throws Throwable {
        return (Hash)obj.callMethod("mapBulk", rec, crec);
    }
}

