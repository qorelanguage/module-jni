/** Java wrapper for the %Qore BulkUpsertOperation class
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

//! base class for bulk DML upsert operations
/** This class assists with bulk upsert (SQL merge) operations into a target @ref SqlUtil::AbstractTable "table".

    @par Submitting Data
    To use this class, queue data in the form of a hash (a single row or a set of rows) or a list of rows
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
    BulkUpsertOperation op1 = new BulkUpsertOperation(table1);
    BulkUpsertOperation op2 = new BulkUpsertOperation(table2);

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
public class BulkUpsertOperation extends AbstractBulkOperation {
    //! creates the object from the supplied arguments
    /** @param target the target table object
        @param opts an optional hash of options for the object as follows:
        - \c "block_size": the number of rows executed at once (default: 1000)
        - \c "info_log": an optional info logging callback; must accept a string format specifier and sprintf()-style arguments
        - \c "upsert_strategy": the upsert strategy to use; default SqlUtil::AbstractTable::UpsertAuto; see @ref upsert_options for possible values for the upsert strategy
    */
    public BulkUpsertOperation(AbstractTable target, Map<String, Object> opts) throws Throwable {
        super(QoreJavaApi.newObjectSave("BulkSqlUtil::BulkUpsertOperation", target.getQoreObject(), opts));
    }

    //! creates the object from the supplied arguments
    /** @param target the target table object
    */
    public BulkUpsertOperation(AbstractTable target) throws Throwable {
        super(QoreJavaApi.newObjectSave("BulkSqlUtil::BulkUpsertOperation", target.getQoreObject()));
    }
}
