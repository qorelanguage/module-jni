/** Java wrapper for the %Qore AbstractBulkOperation class
 *
 */
package org.qore.lang.bulksqlutil;

// jni module imports
import org.qore.jni.QoreJavaApi;
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;

// Qore imports
import org.qore.lang.AbstractDatasource;
import org.qore.lang.sqlutil.AbstractTable;

// java imports
import java.util.Map;

//! Java wrapper for the base %Qore class for bulk DML operations
/** This class provides the majority of the API support for bulk DML operations for the concrete child classes that
    inherit it.

    @par Submitting Data
    To use this class's API, queue data in the form of a hash (a single row or a set of rows) or a list of rows
    by calling the queueData() method.\n\n
    The queueData() method queues data to be written to the database; the queue is flush()ed
    automatically when \c block_size rows have been queued.

    @par Flushing and Discarding Data
    Each call to flush() (whether implicit or explicit) will cause a single call to be made to
    the dataserver; all queued rows are sent in a single bulk DML call, which allows for efficient
    processing of large amounts of data.\n\n
    A call to flush() must be made before committing the transaction to ensure that any remaining
    rows in the internal queue have been written to the database.  Because the destructor() will
    throw an exception if any data is left in the internal queue when the object is destroyed, a call
    to discard() must be made prior to the destruction of the object in case of errors.

    @code{.java}
// single commit and rollback
try {
    // data is queued and flushed automatically when the buffer is full
    for (Map<String, Object> i : data1) {
        op1.queueData(i);
    }
    for (Map<String, Object> i : data2) {
        op2.queueData(i);
    }

    // each operation needs to be flushed or discarded individually
    op1.flush();
    op2.flush();
    ds.commit();
} catch (Throwable e) {
    op1.discard();
    op2.discard();
    ds.rollback();
    throw e;
}
    @endcode

    @note
    - Each bulk DML object must be manually flush()ed before committing or manually
      discard()ed before rolling back to ensure that all data is managed properly in the same
      transaction and to ensure that no exception is thrown in the destructor().
      See the example above for more information.
    - If the underlying driver does not support bulk operations, then such support is
      emulated with single SQL operations; in such cases performance will be reduced.  Call
      @ref SqlUtil::AbstractTable::hasArrayBind() to check at runtime if the driver supports
      bulk SQL operations.
    - loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.BulkSqlUtil.AbstractBulkOperation;</tt>
*/
@Deprecated
public class AbstractBulkOperation extends QoreObjectWrapper {
    // static initialization
    static {
        // loads and initializes the Qore library and the jni module (if necessary) and loads the \c BulkSqlUtil module
        try {
            QoreJavaApi.initQore();
            QoreJavaApi.callFunction("load_module", "BulkSqlUtil");
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates a new AbstractBulkOperation object wrapping the Qore object
    public AbstractBulkOperation(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! queues row data in the block buffer; the block buffer is flushed to the DB if the buffer size reaches the limit defined by the \c block_size option; does not commit the transaction
    /** @par Example:
        @code{.java}
// single commit and rollback
try {
    // data is queued and flushed automatically when the buffer is full
    for (Map<String, Object> i : data1) {
        op1.queueData(i);
    }
    for (Map<String, Object> i : data2) {
        op2.queueData(i);
    }

    // each operation needs to be flushed or discarded individually
    op1.flush();
    op2.flush();
    ds.commit();
} catch (Throwable e) {
    op1.discard();
    op2.discard();
    ds.rollback();
    throw e;
}
        @endcode

        @param data the input record or record set in case a hash of lists is passed; each hash represents a row (keys are column names and values are column values); when inserting, @ref sql_iop_funcs can also be used.  If at least one hash value is a list, then any non-hash (indicating an @ref sql_iop_funcs "insert opertor hash") and non-list values will be assumed to be constant values for every row and therefore future calls of this method (and overloaded variants) will ignore any values given for such keys and use the values given in the first call.

        @note
        - the first row passed is taken as a template row; every other row must always have the same keys in the same order, otherwise the results are unpredictable
        - if any @ref sql_iop_funcs are used, then they are assumed to be identical in every row
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction or destroying the object when using this method
        - flush() or discard() needs to be executed individually for each bulk operation object used in the block whereas the DB transaction needs to be committed or rolled back once per datasource

        @see
        - flush()
        - discard()
    */
    public void queueData(Map<String, Object> data) throws Throwable {
        obj.callMethod("queueData", data);
    }

    //! queues row data in the block buffer; the block buffer is flushed to the DB if the buffer size reaches the limit defined by the \c block_size option; does not commit the transaction
    /** @par Example:
        @code{.java}
// single commit and rollback
try {
    // data is queued and flushed automatically when the buffer is full
    op1.queueData(l1);
    op2.queueData(l2);

    // each operation needs to be flushed or discarded individually
    op1.flush();
    op2.flush();
    ds.commit();
} catch (Throwable e) {
    op1.discard();
    op2.discard();
    ds.rollback();
    throw e;
}
        @endcode

        @param l a list of hashes representing the input row data; each hash represents a row (keys are column names and values are column values); when inserting, @ref sql_iop_funcs can also be used

        @note
        - the first row passed is taken as a template row; every other row must always have the same keys in the same order, otherwise the results are unpredictable
        - if any @ref sql_iop_funcs are used, then they are assumed to be identical in every row
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction or destroying the object when using this method
        - flush() or discard() needs to be executed individually for each bulk operation object used in the block whereas the DB transaction needs to be committed or rolled back once per datasource

        @see
        - flush()
        - discard()
    */
    public void queueData(Map<String, Object>[] l) throws Throwable {
        obj.callMethod("queueData", (Object)l);
    }

    //! flushes any remaining batched data to the database; this method should always be called before committing the transaction or destroying the object
    /** @par Example:
        @code{.java}
// single commit and rollback
try {
    // data is queued and flushed automatically when the buffer is full
    for (Map<String, Object> i : data1) {
        op1.queueData(i);
    }
    for (Map<String, Object> i : data2) {
        op2.queueData(i);
    }

    // each operation needs to be flushed or discarded individually
    op1.flush();
    op2.flush();
    ds.commit();
} catch (Throwable e) {
    op1.discard();
    op2.discard();
    ds.rollback();
    throw e;
}
        @endcode

        @note
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction or destroying the object when using this method
        - flush() or discard() needs to be executed individually for each bulk operation object used in the block whereas the DB transaction needs to be committed or rolled back once per datasource

        @see
        - queueData()
        - discard()
    */
    public void flush() throws Throwable {
        obj.callMethod("flush");
    }

    //! discards any buffered batched data; this method should be called before destroying the object if an error occurs
    /** @par Example:
        @code{.java}
// single commit and rollback
try {
    // data is queued and flushed automatically when the buffer is full
    for (Map<String, Object> i : data1) {
        op1.queueData(i);
    }
    for (Map<String, Object> i : data2) {
        op2.queueData(i);
    }

    // each operation needs to be flushed or discarded individually
    op1.flush();
    op2.flush();
    ds.commit();
} catch (Throwable e) {
    op1.discard();
    op2.discard();
    ds.rollback();
    throw e;
}
        @endcode

        @note
        - make sure to call flush() before committing the transaction or discard() before rolling back the transaction or destroying the object when using this method
        - flush() or discard() needs to be executed individually for each bulk operation object used in the block whereas the DB transaction needs to be committed or rolled back once per datasource

        @see
        - queueData()
        - flush()
    */
    public void discard() throws Throwable {
        obj.callMethod("discard");
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

    //! returns the underlying SqlUtil::AbstractTable object
    public AbstractTable getTable() throws Throwable {
        // callMethodSave() is not needed here as the strong reference to the
        // Qore object is stored in the current object
        return new AbstractTable((QoreObject)obj.callMethod("getTable"));
    }

    //! returns the @ref Qore::SQL::AbstractDatasource "AbstractDatasource" object associated with this object
    public AbstractDatasource getDatasource() throws Throwable {
        // callMethodSave() is not needed here as the strong reference to the
        // Qore object is stored in the current object
        return new AbstractDatasource((QoreObject)obj.callMethod("getDatasource"));
    }

    //! returns the affected row count
    public int getRowCount() throws Throwable {
        return ((Long)obj.callMethod("getRowCount")).intValue();
    }
}
