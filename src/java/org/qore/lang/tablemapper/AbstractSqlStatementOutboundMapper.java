/** Java wrapper for the %Qore AbstractSqlStatementOutboundMapper class
 *
 */
package org.qore.lang.tablemapper;

// qorus imports
import org.qore.lang.mapper.Mapper;

// java imports
import java.util.Map;
import java.util.HashMap;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref TableMapper::AbstractSqlStatementOutboundMapper class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class AbstractSqlStatementOutboundMapper extends Mapper {
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
    public AbstractSqlStatementOutboundMapper(QoreObject obj) {
        super(obj);
    }

    //! commits the transaction and frees the Qore::SQL::AbstractDatasource transaction thread resource
    public void commit() throws Throwable {
        obj.callMethod("commit");
    }

    //! rolls the transaction back and frees the Qore::SQL::AbstractDatasource transaction thread resource
    public void rollback() throws Throwable {
        obj.callMethod("rollback");
    }

    //! Retrieve mapped data as a hash of lists.
    /** @return *hash with data or null in case there are no more data available.

        The size of the batch is driven by the \c select_block option passed in the constructor.

        The hash is in Qore::SQL::AbstractDatasource::select() form - meaning it is a hash
        with column names as keys. Values are lists of column values.
        This data structure is used for Qore::context statement or BulksSqlUtil
        operations.
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getData() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getData");
    }

    //! Retrieve mapped data as a list of hashes.
    /** @return list with data or null in case there are no more data available.

        Size of the batch is driven by the \c select_block option passed in the constructor.

        List is in Qore::SQL::AbstractDatasource::selectRows() form - meaning it is a list
        with hashes, where every hash has column names as keys with single values as
        hash values.
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] getDataRows() throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("getDataRows");
    }
}
