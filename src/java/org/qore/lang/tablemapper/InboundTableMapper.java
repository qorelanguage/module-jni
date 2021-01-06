/** Java wrapper for the %Qore InboundTableMapper class
 *
 */
package org.qore.lang.tablemapper;

// qorus imports
import org.qore.lang.mapper.Mapper;
import org.qore.lang.AbstractDatasource;
import org.qore.lang.sqlutil.AbstractTable;

// java imports
import java.util.Map;
import java.util.HashMap;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref TableMapper::InboundTableMapper class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.TableMapper.InboundTableMapper;</tt>
*/
@Deprecated
public class InboundTableMapper extends Mapper {
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
    public InboundTableMapper(QoreObject obj) {
        super(obj);
    }

    //! inserts or upserts a row into the target table based on a mapped input record; does not commit the transaction
    /** @param rec the input record

        @return a hash of the row values inserted/upserted (row name: value); note that any sequence values inserted are also returned here

        @note on mappers with "insert_block > 1" (i.e. also the underlying DB must allow for bulk operations), it is not allowed to use both single-record insertions (like insertRow()) and bulk operations (queueData()) in one transaction. Mixing insertRow() with queueData() leads to mismatch of Oracle DB types raising corresponding exceptions depending on particular record types (e.g. "ORA-01722: invalid number").

        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> insertRow(Map<String, Object> rec) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("insertRow", rec);
    }

    //! inserts/upserts a row (or a set of rows, in case a Map<String, Object> of lists is passed) into the block buffer based on a mapped input record; the block buffer is flushed to the DB if the buffer size reaches the limit defined by the \c "insert_block" option; does not commit the transaction
    /** @par Example:
        @code{.py}
{
    AbstractTable table = UserApi.get_sql_table("some_table");
    try {
        InboundTableMapper table_mapper = UserApi.get_mapper("my_mapper");
        try {
            table_mapper.queueData(recs, crec);
            table_mapper.flush();
        } catch (Throwable e) {
            table_mapper.discard();
        } finally {
            table_mapper.release();
        }
    } finally {
        table.release();
    }
}
        @endcode

        Data is only inserted/upserted if the block buffer size reaches the limit defined by the \c "insert_block" option,
        in which case this method returns all the data inserted/upserted.  In case the mapped data is only inserted into
        the cache, no value is returned.

        @param rec the input record or record set in case a Map<String, Object> of lists is passed
        @param crec an optional simple Map<String, Object> (may be null) of data to be added to each input row before mapping

        @return if batch data was inserted then a Map<String, Object> (columns) of lists (row data) of all data inserted
        and potentially returned (in case of sequences) from the database server is returned; if constant mappings
        are used with batch data, then they are returned as single values assigned to the Map<String, Object> keys; always
        returns a batch (Map<String, Object> of lists); in case no insert was made, null is returned

        @note
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction
          or destroying the object when using this method
        - flush() or discard() needs to be executed for each mapper used in the block when using multiple mappers
          whereas the DB transaction needs to be committed or rolled back once per datasource
        - this method and batched inserts/upserts in general cannot be used when the \c "unstable_input" option is
          given in the constructor
        - if the \c "insert_block" option is set to 1, then this method simply calls insertRow(); however please note
          that in this case the return value is a Map<String, Object> of single value lists corresponding to a batch data insert
        - if an error occurs flushing data, the count is reset by calling
          @ref Mapper::Mapper::resetCount() "Mapper::resetCount()"
        - using a Map<String, Object> of lists in \a rec; note that this provides very high performance with SQL drivers that
          support Bulk DML
        - in case a Map<String, Object> of empty lists is passed, null is returned
        - 'crec' does not affect the number of output lines; in particular, if 'rec' is a batch with \c N rows of a
          column \c C and 'crec = ("C" : "mystring")' then the output will be as if there was 'N' rows with
          \c C = "mystring" on the input.
        - on mappers with "insert_block > 1" (i.e. also the underlying DB must allow for bulk operations), it is not
          allowed to use both single-record insertions (like insertRow()) and bulk operations (queueData()) in one
          transaction. Mixing insertRow() with queueData() leads to mismatch of Oracle DB types raising corresponding
          exceptions depending on particular record types (e.g. "ORA-01722: invalid number").

        @see
        - flush()
        - discard()

        @throw MAPPER-BATCH-ERROR this exception is thrown if this method is called when the \c "unstable_input" option
        was given in the constructor
        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> queueData(Map<String, Object> rec, Map<String, Object> crec) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("queueData", rec, crec);
    }

    //! inserts/upserts a row (or a set of rows, in case a Map<String, Object> of lists is passed) into the block buffer based on a mapped input record; the block buffer is flushed to the DB if the buffer size reaches the limit defined by the \c "insert_block" option; does not commit the transaction
    /** @par Example:
        @code{.java}
{
    AbstractTable table = UserApi.get_sql_table("some_table");
    try {
        table_mapper = UserApi.get_mapper("my_mapper");
        try {
            table_mapper.queueData(recs);
            table_mapper.flush();
        } catch (Throwable e) {
            table_mapper.discard();
        } finally {
            table_mapper.release();
        }
    } finally {
        table.release();
    }
}
        @endcode

        Data is only inserted/upserted if the block buffer size reaches the limit defined by the \c "insert_block" option,
        in which case this method returns all the data inserted/upserted.  In case the mapped data is only inserted into
        the cache, no value is returned.

        @param rec the input record or record set in case a Map<String, Object> of lists is passed

        @return if batch data was inserted then a Map<String, Object> (columns) of lists (row data) of all data inserted
        and potentially returned (in case of sequences) from the database server is returned; if constant mappings
        are used with batch data, then they are returned as single values assigned to the Map<String, Object> keys; always
        returns a batch (Map<String, Object> of lists); in case no insert was made, null is returned

        @note
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction
          or destroying the object when using this method
        - flush() or discard() needs to be executed for each mapper used in the block when using multiple mappers
          whereas the DB transaction needs to be committed or rolled back once per datasource
        - this method and batched inserts/upserts in general cannot be used when the \c "unstable_input" option is
          given in the constructor
        - if the \c "insert_block" option is set to 1, then this method simply calls insertRow(); however please note
          that in this case the return value is a Map<String, Object> of single value lists corresponding to a batch data insert
        - if an error occurs flushing data, the count is reset by calling
          @ref Mapper::Mapper::resetCount() "Mapper::resetCount()"
        - using a Map<String, Object> of lists in \a rec; note that this provides very high performance with SQL drivers that
          support Bulk DML
        - in case a Map<String, Object> of empty lists is passed, null is returned
        - 'crec' does not affect the number of output lines; in particular, if 'rec' is a batch with \c N rows of a
          column \c C and 'crec = ("C" : "mystring")' then the output will be as if there was 'N' rows with
          \c C = "mystring" on the input.
        - on mappers with "insert_block > 1" (i.e. also the underlying DB must allow for bulk operations), it is not
          allowed to use both single-record insertions (like insertRow()) and bulk operations (queueData()) in one
          transaction. Mixing insertRow() with queueData() leads to mismatch of Oracle DB types raising corresponding
          exceptions depending on particular record types (e.g. "ORA-01722: invalid number").

        @see
        - flush()
        - discard()

        @throw MAPPER-BATCH-ERROR this exception is thrown if this method is called when the \c "unstable_input" option
        was given in the constructor
        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> queueData(Map<String, Object> rec) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("queueData", rec);
    }

    //! inserts/upserts a set of rows (list of hashes representing input records) into the block buffer based on a mapped input record; the block buffer is flushed to the DB if the buffer size reaches the limit defined by the \c "insert_block" option; does not commit the transaction
    /** @par Example:
        @code{.java}
{
    AbstractTable table = UserApi.get_sql_table("some_table");
    try {
        table_mapper = UserApi.get_mapper("my_mapper");
        try {
            table_mapper.queueData(l, crec);
            table_mapper.flush();
        } catch (Throwable e) {
            table_mapper.discard();
        } finally {
            table_mapper.release();
        }
    } finally {
        table.release();
    }
}
        @endcode

        Data is only inserted/upserted if the block buffer size reaches the limit defined by the \c "insert_block" option, in which case this method returns all the data inserted/upserted.  In case the mapped data is only inserted into the cache, no value is returned.

        @param l a list of hashes representing the input records
        @param crec an optional simple Map<String, Object> of data to be added to each row

        @return if batch data was inserted/upserted then a Map<String, Object> (columns) of lists (row data) of all data inserted/upserted and potentially returned (in case of sequences) from the database server is returned

        @note
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction or destroying the object when using this method
        - flush() or discard() needs to be executed for each mapper used in the block when using multiple mappers whereas the DB transaction needs to be committed or rolled back once per datasource
        - this method and batched inserts/upserts in general cannot be used when the \c "unstable_input" option is given in the constructor
        - if the \c "insert_block" option is set to 1, then this method simply calls insertRow()
        - if an error occurs flushing data, the count is reset by calling @ref Mapper::Mapper::resetCount() "Mapper::resetCount()"

        @see
        - flush()
        - discard()

        @throw MAPPER-BATCH-ERROR this exception is thrown if this method is called when the \c "unstable_input" option was given in the constructor
        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> queueData(Map<String, Object>[] l, Map<String, Object> crec) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("queueData", (Object)l, crec);
    }

    //! inserts/upserts a set of rows (list of hashes representing input records) into the block buffer based on a mapped input record; the block buffer is flushed to the DB if the buffer size reaches the limit defined by the \c "insert_block" option; does not commit the transaction
    /** @par Example:
        @code{.java}
{
    AbstractTable table = UserApi.get_sql_table("some_table");
    try {
        table_mapper = UserApi.get_mapper("my_mapper");
        try {
            table_mapper.queueData(l);
            table_mapper.flush();
        } catch (Throwable e) {
            table_mapper.discard();
        } finally {
            table_mapper.release();
        }
    } finally {
        table.release();
    }
}
        @endcode

        Data is only inserted/upserted if the block buffer size reaches the limit defined by the \c "insert_block" option, in which case this method returns all the data inserted/upserted.  In case the mapped data is only inserted into the cache, no value is returned.

        @param l a list of hashes representing the input records

        @return if batch data was inserted/upserted then a Map<String, Object> (columns) of lists (row data) of all data inserted/upserted and potentially returned (in case of sequences) from the database server is returned; returns null if no data was inserted

        @note
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction or destroying the object when using this method
        - flush() or discard() needs to be executed for each mapper used in the block when using multiple mappers whereas the DB transaction needs to be committed or rolled back once per datasource
        - this method and batched inserts/upserts in general cannot be used when the \c "unstable_input" option is given in the constructor
        - if the \c "insert_block" option is set to 1, then this method simply calls insertRow()
        - if an error occurs flushing data, the count is reset by calling @ref Mapper::Mapper::resetCount() "Mapper::resetCount()"

        @see
        - flush()
        - discard()

        @throw MAPPER-BATCH-ERROR this exception is thrown if this method is called when the \c "unstable_input" option was given in the constructor
        @throw MISSING-INPUT a field marked mandatory is missing
        @throw STRING-TOO-LONG a field value exceeds the maximum value and the 'trunc' key is not set
        @throw INVALID-NUMBER the field is marked as numeric but the input value contains non-numeric data
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> queueData(Map<String, Object>[] l) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("queueData", (Object)l);
    }

    //! flushes any remaining batched data to the database; this method should always be called before committing the transaction or destroying the object
    /** @par Example:
        @code{.java}
{
    AbstractTable table = UserApi.get_sql_table("some_table");
    try {
        table_mapper = UserApi.get_mapper("my_mapper");
        try {
            table_mapper.queueData(l);
            table_mapper.flush();
        } catch (Throwable e) {
            table_mapper.discard();
        } finally {
            table_mapper.release();
        }
    } finally {
        table.release();
    }
}
        @endcode

        @return if batch data was inserted then a hash (columns) of lists (row data) of all data inserted and potentially
        returned (in case of sequences) from the database server is returned; if constant mappings are used with batch
        data, then they are returned as single values assigned to the hash keys; returns null if no data was inserted

        @note
        - flush() or discard() needs to be executed for each mapper used in the block when using multiple mappers whereas the DB transaction needs to be committed or rolled back once per datasource
        - also clears any row @ref closure or @ref call_reference set for batch operations
        - if an error occurs flushing data, the count is reset by calling @ref Mapper::Mapper::resetCount() "Mapper::resetCount()"

        @see
        - queueData()
        - discard()
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> flush() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("flush");
    }

    //! discards any buffered batched data; this method should be called after using the batch APIs (queueData()) and an error occurs
    /** @par Example:
        @code{.java}
{
    AbstractTable table = UserApi.get_sql_table("some_table");
    try {
        table_mapper = UserApi.get_mapper("my_mapper");
        try {
            table_mapper.queueData(l);
            table_mapper.flush();
        } catch (Throwable e) {
            table_mapper.discard();
        } finally {
            table_mapper.release();
        }
    } finally {
        table.release();
    }
}
        @endcode

        @note
        - flush() or discard() needs to be executed for each mapper used in the block when using multiple mappers whereas the DB transaction needs to be committed or rolled back once per datasource
        - also clears any row @ref closure or @ref call_reference set for batch operations
        - if an error occurs flushing data, the count is reset by calling @ref Mapper::Mapper::resetCount() "Mapper::resetCount()"

        @see
        - queueData()
        - flush()
    */
    public void discard() throws Throwable {
        obj.callMethod("discard");
    }

    //! returns a list argument for the SqlUtil "returning" option, if applicable
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] getReturning() throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("getReturning");
    }

    //! flushes any queued data and commits the transaction
    public void commit() throws Throwable {
        obj.callMethod("commit");
    }

    //! discards any queued data and rolls back the transaction
    public void rollback() throws Throwable {
        obj.callMethod("rollback");
    }

    //! returns the table name
    public String getTableName() throws Throwable {
        return (String)obj.callMethod("getTableName");
    }

    //! returns the underlying @ref org.qore.lang.sqlutil.AbstractTable object
    public AbstractTable getTable() throws Throwable {
        // callMethodSave() is not needed here as the strong reference to the
        // Qore object is stored in the current object
        return new AbstractTable((QoreObject)obj.callMethod("getTable"));
    }

    //! returns the @ref org.qore.lang.AbstractDatasource object associated with this object
    public AbstractDatasource getDatasource() throws Throwable {
        // callMethodSave() is not needed here as the strong reference to the
        // Qore object is stored in the current object
        return new AbstractDatasource((QoreObject)obj.callMethod("getDatasource"));
    }
}
