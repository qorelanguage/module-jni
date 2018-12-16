/** Java wrapper for the %Qore BulkInsertOperation class
 *
 */
package org.qore.lang.bulksqlutil;

// jni module imports
import org.qore.jni.QoreJavaApi;

// Qore imports
import org.qore.lang.AbstractDatasource;
import org.qore.lang.sqlutil.AbstractTable;
import org.qore.lang.bulksqlutil.BulkRowCallback;

// java imports
import java.util.Map;

//! Java wrapper for the Qore class for bulk DML insert operations
/** This class assists with bulk inserts into a target @ref org.qore.lang.sqlutil.AbstractTable "table".

    @par Submitting Data
    To use this class, queue data in the form of a hash (a single row or a set of rows) or a list of rows
    by calling the queueData() method.\n\n
    The queueData() method queues data to be written to the database; the queue is flush()ed
    automatically when \c block_size rows have been queued.

    @par Retrieving Data From Inserts
    It is possible to use @ref sql_iop_funcs in the hashes submitted with queueData(); in this case the
    BulkInsertOperation class assumes that every row has the same operations as in the first row.
    Output data can then be processed by using the \c rowcode option in the constructor() or by calling
    setRowCode().\n\n
    In case @ref sql_iop_funcs are used and a \c rowcode option is set, then the SQL DML query for inserts
    is creating using the \c "returning" @ref SqlUtil::AbstractTable::InsertOptions "insert option", therefore
    the DBI driver in this case must support this option as well.

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
    BulkInsertOperation op1 = new BulkInsertOperation(table1);
    BulkInsertOperation op2 = new BulkInsertOperation(table2);

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

    @note Each bulk DML object must be manually flush()ed before committing or manually
    discard()ed before rolling back to ensure that all data is managed properly in the same
    transaction and to ensure that no exception is thrown in Qore destructor.
    See the example above for more information.
*/
public class BulkInsertOperation extends AbstractBulkOperation {
    //! creates the object from the supplied arguments
    /** @param target the target table object
        @param opts an optional hash of options for the object as follows:
        - \c "info_log": an optional info logging callback; must accept a string format specifier and sprintf()-style
          arguments
        - \c "block_size": the number of rows executed at once (default: 1000)
        - \c "rowcode": a per-row callback that must be an instance of
          @ref org.qore.lang.bulksqlutil.BulkRowCallback "BulkRowCallback"; the
          @ref org.qore.lang.bulksqlutil.BulkRowCallback.call() "BulkRowCallback.call()" method will be called for
          every row after a bulk insert; the hash argument representing the row inserted will also contain any output
          values if applicable (for example if @ref sql_iop_funcs are used in the row hashes submitted to queueData())

        @see setRowCode()
    */
    public BulkInsertOperation(AbstractTable target, Map<String, Object> opts) throws Throwable {
        super(QoreJavaApi.newObjectSave("BulkSqlUtil::BulkInsertOperation", target.getQoreObject(), opts));
    }

    //! creates the object from the supplied arguments
    /** @param target the target table object

        @see setRowCode()
    */
    public BulkInsertOperation(AbstractTable target) throws Throwable {
        super(QoreJavaApi.newObjectSave("BulkSqlUtil::BulkInsertOperation", target.getQoreObject()));
    }

    //! sets a row callback that will be called when data has been sent to the database and all output data is available; must accept a hash argument that represents the data written to the database including any output arguments. This code will be reset once the transaction is commited.
    /** @par Example:
        @code{.java}
// single commit and rollback
try {
    inserter.setRowCode(new MyRowCallback());

    // data is queued and flushed automatically when the buffer is full
    for (Map<String, Object> i : data) {
        inserter.queueData(i);
    }

    // each operation needs to be flushed or discarded individually
    inserter.flush();
    ds.commit();
} catch (Throwable e) {
    inserter.discard();
    ds.rollback();
    throw e;
}
        @endcode

        @param rowc a row callback that will be called when data has been sent to the database and all output data is
        available; must accept a hash argument that represents the data written to the database including any output
        arguments

        @note
        - the row callback can also be set by using the \c "rowcode" option in the constructor()
        - if this method is not called before the first row is queued then output values will not be retrieved; the
          initial query is built when the template row is queued and output values are only retrieved if a \c rowcode
          callback is set beforehand
    */
    public void setRowCode(BulkRowCallback rowc) throws Throwable {
        obj.callMethod("setRowCode", rowc);
    }

    //! clears the row callback called when data has been sent to the database and all output data is available
    /** @par Example:
        @code{.java}
// single commit and rollback
try {
    // clear any row callback
    inserter.setRowCode();

    // data is queued and flushed automatically when the buffer is full
    for (Map<String, Object> i : data) {
        inserter.queueData(i);
    }

    // each operation needs to be flushed or discarded individually
    inserter.flush();
    ds.commit();
} catch (Throwable e) {
    inserter.discard();
    ds.rollback();
    throw e;
}
        @endcode
    */
    public void setRowCode() throws Throwable {
        obj.callMethod("setRowCode");
    }
}
