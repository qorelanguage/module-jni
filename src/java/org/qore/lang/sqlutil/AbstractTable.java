/** Java wrapper for the %Qore AbstractTable class
 *
 */
package org.qore.lang.sqlutil;

// java imports
import java.util.Map;
import java.util.HashMap;
import java.util.Collections;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreJavaApi;
import org.qore.lang.AbstractDatasource;

// Qore imports
import org.qore.lang.sqlutil.AbstractSqlUtilBase;
import org.qore.lang.AbstractSQLStatement;

//! Java wrapper for the @ref SqlUtil::AbstractTable class in %Qore
public class AbstractTable extends AbstractSqlUtilBase {
    /** @defgroup upsert_options Upsert Strategy Codes
        These options are used with:
        - @ref upsert()
        - @ref upsertFromSelect()

        to specify the upsert strategy when synchronizing table data
        */
    //@{
    //! Upsert option: insert first, if the insert fails, then update
    /** with this option an insert is attempted, and if it fails due to a duplicate row, then an update is made unconditionally; with
        this upsert strategy, the following row result codes are possible:
        - @ref UR_Inserted
        - @ref UR_Verified (the only value returned by certain drivers with optimized upsert implementations)
    */
    public static int UpsertInsertFirst = 1;

    //! Upsert option: update first, if the update fails, then insert
    /** with this option an update is attempted, and if it fails due to a missing row, then an insert is performed; with
        this upsert strategy, the following row result codes are possible:
        - @ref UR_Inserted
        - @ref UR_Verified (the only value returned by certain drivers with optimized upsert implementations)
    */
    public static int UpsertUpdateFirst = 2;

    //! Upsert option: select first, if the row is unchanged, do nothing, if it doesn't exist, insert, otherwise update
    /** with this option the row is selected, if it doesn't exist, an insert is made, and an update is made only if the
        values are different; with this upsert strategy, the following row result codes are possible:
        - @ref UR_Inserted
        - @ref UR_Updated
        - @ref UR_Unchanged
    */
    public static int UpsertSelectFirst = 3;

    //! Upsert option: if the target table is empty, use @ref UpsertInsertFirst, otherwise use @ref UpsertUpdateFirst
    /** With this upsert strategy, the following row result codes are possible:
        - @ref UR_Inserted
        - @ref UR_Verified (the only value returned by certain drivers with optimized upsert implementations)
    */
    public static int UpsertAuto = 4;

    //! Upsert option: insert if the row does not exist, otherwise ignore
    /** With this upsert strategy, the following row result codes are possible:
        - @ref UR_Inserted
        - @ref UR_Unchanged
    */
    public static int UpsertInsertOnly = 5;

    //! Upsert option: update if the row exists, otherwise ignore
    /** With this upsert strategy, the following row result codes are possible:
        - @ref UR_Verified
        - @ref UR_Unchanged
    */
    public static int UpsertUpdateOnly = 6;

    //! hash mapping upsert strategy codes to a text description
    /** @see @ref UpsertStrategyDescriptionMap for a reverse mapping
    */
    public static final Map<Integer, String> UpsertStrategyMap = Collections.unmodifiableMap(new HashMap<Integer, String>() {
        {
            put(UpsertInsertFirst, "UpsertInsertFirst");
            put(UpsertUpdateFirst, "UpsertUpdateFirst");
            put(UpsertSelectFirst, "UpsertSelectFirst");
            put(UpsertAuto, "UpsertAuto");
            put(UpsertInsertOnly, "UpsertInsertOnly");
            put(UpsertUpdateOnly, "UpsertUpdateOnly");
        }
    });

    //! hash mapping upsert strategy descriptions to upsert strategy codes
    /** @see @ref UpsertStrategyMap for a reverse mapping
    */
    public static final Map<String, Integer> UpsertStrategyDescriptionMap = Collections.unmodifiableMap(new HashMap<String, Integer>() {
        {
            put("UpsertInsertFirst", UpsertInsertFirst);
            put("UpsertUpdateFirst", UpsertUpdateFirst);
            put("UpsertSelectFirst", UpsertSelectFirst);
            put("UpsertAuto", UpsertAuto);
            put("UpsertInsertOnly", UpsertInsertOnly);
            put("UpsertUpdateOnly", UpsertUpdateOnly);
        }
    });
    //@}

    /** @defgroup upsert_results Upsert Result Codes
        @see @ref UpsertResultMap and @ref UpsertResultDescriptionMap
        */
    //@{
    //! row was inserted
    public static int UR_Inserted = 1;

    //! row was updated unconditionally (not returned with @ref UpsertSelectFirst)
    public static int UR_Verified = 2;

    //! row was updated because it was different (only possible with @ref UpsertSelectFirst)
    public static int UR_Updated = 3;

    //! row was unchanged (only possible with @ref UpsertSelectFirst, @ref UpsertInsertOnly, and @ref UpsertUpdateOnly)
    public static int UR_Unchanged = 4;

    //! row was deleted (only possible with batch upsert methods such as @ref upsertFromIterator() where @ref SqlUtil::AbstractTable::UpsertOptions "upsert option" \c delete_others is true)
    public static int UR_Deleted = 5;
    //@}

    //! hash mapping upsert results to a description
    /** @see @ref UpsertResultDescriptionMap for a reverse mapping
    */
    public static final Map<Integer, String> UpsertResultMap = Collections.unmodifiableMap(new HashMap<Integer, String>() {
        {
            put(UR_Inserted, "inserted");
            put(UR_Verified, "verified");
            put(UR_Updated, "updated");
            put(UR_Unchanged, "unchanged");
            put(UR_Deleted, "deleted");
        }
    });

    //! hash mapping upsert descriptions to codes
    /** @see @ref UpsertResultMap for a reverse mapping
    */
    public static final Map<String, Integer> UpsertResultDescriptionMap = Collections.unmodifiableMap(new HashMap<String, Integer>() {
        {
            put("inserted", UR_Inserted);
            put("verified", UR_Verified);
            put("updated", UR_Updated);
            put("unchanged", UR_Unchanged);
            put("deleted", UR_Deleted);
        }
    });

    //! maps upsert result codes to single letter symbols
    public static final Map<Integer, String> UpsertResultLetterMap = Collections.unmodifiableMap(new HashMap<Integer, String>() {
        {
            put(UR_Inserted, "I");
            put(UR_Verified, "V");
            put(UR_Updated, "U");
            put(UR_Unchanged, ".");
            put(UR_Deleted, "X");
        }
    });

    //! creates the object as a wrapper for the Qore object
    public AbstractTable(QoreObject obj) {
        super(obj);
    }

    //! returns the name of the table
    public String getName() throws Throwable {
        return (String)obj.callMethod("getName");
    }

    //! finds a row in the table with the given primary key value; if no row matches the primary key value passed then null is returned
    /** @par Example:
        @code{.java}
HashMap<String, Object> row = table.find(id);
        @endcode

        @throw PRIMARY-KEY-ERROR the table has no primary key or the primary key has more than one column
    */
    @SuppressWarnings("unchecked")
    HashMap<String, Object> find(Object id) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("find", id);
    }

    //! finds rows in the table with the given primary key values; if no row matches any primary key value passed then null is returned
    /** @par Example:
        @code{.java}
HashMap<String, Object>[] rows = table.find(list);
        @endcode

        @param ids the list of primary key IDs to find; if the list is empty then null is returned

        @return a list of hashes of rows matching the primary key IDs passed or null if no row matches any primary key value passed

        @throw PRIMARY-KEY-ERROR the table has no primary key or the primary key has more than one column
    */
    @SuppressWarnings("unchecked")
    HashMap<String, Object>[] find(Object[] ids) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("find", (Object)ids);
    }

    //! finds a row in the table with the given primary key value given as a hash; if no row matches the primary key value passed then null is returned
    /** @par Example:
        @code{.java}
HashMap<String, Object> row = table.find(row);
        @endcode

        @param row a hash giving the primary key value to find; other columns may also appear in the hash, however at least all columns of the primary key must be present

        @return a hash of the row value matching the primary key value passed or null if no row matches the primary key value passed

        @throw PRIMARY-KEY-ERROR the table has no primary key or the the hash passed does not contain all columns of the primary key

        @note a table with a primary key with a single column can also be used with this method; just pass a hash with one key
    */
    @SuppressWarnings("unchecked")
    HashMap<String, Object> find(Map<String, Object> row) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("find", row);
    }

    //! finds a single row in the table that match the row condition passed; multiple rows may match, but only one row will be returned from the database; if no row matches the condition hash passed then null is returned
    /** @par Example:
        @code{.java}
HashMap<String, Object> row = table.findSingle(h);
        @endcode

        @param cond a hash giving the column values to find; see @ref where_clauses for the format of this argument

        @return a hash representing a single row in the table with the given column values; multiple rows may match, but only one row will be returned from the database; if no row matches the condition hash passed then null is returned

        @throw WHERE-ERROR unknown operator or invalid arguments given in the cond hash for the where clause

        @note this is equivalent to calling selectRow() with \c where = \c cond and \c limit = 1
     */
    @SuppressWarnings("unchecked")
    HashMap<String, Object> findSingle(Map<String, Object> cond) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("findSingle", cond);
    }

    //! reeturns the first row in the table immediately; one and only one row will be returned from the database; if the table is empty, then null is returned
    /** @par Example:
        @code{.java}
HashMap<String, Object> row = table.findSingle();
        @endcode

        @return the first row in the table; if the table is empty, then null is returned

        @note this is equivalent to calling selectRow() with \c limit = 1
     */
    @SuppressWarnings("unchecked")
    HashMap<String, Object> findSingle() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("findSingle");
    }

    //! finds all rows in the table with the given column values; a list of hashes is returned representing the rows returned
    /** @par Example:
        @code{.java}
HashMap<String, Object>[] rows = table.findAll(h);
        @endcode

        @param cond a hash giving the column values to find; see @ref where_clauses for the format of this argument

        @return a list of hashes is returned representing the rows returned

        @throw WHERE-ERROR unknown operator or invalid arguments given in the cond hash for the where clause

        @note this is equivalent to calling selectRows() with \c where = \c cond
    */
    @SuppressWarnings("unchecked")
    HashMap<String, Object>[] findAll(Map<String, Object> cond) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("findAll", cond);
    }

    //! finds all rows in the table with the given column values; a list of hashes is returned representing the rows returned
    /** @par Example:
        @code{.java}
HashMap<String, Object>[] rows = table.findAll();
        @endcode

        @param cond a hash giving the column values to find; see @ref where_clauses for the format of this argument

        @return a list of hashes is returned representing the rows returned

        @throw WHERE-ERROR unknown operator or invalid arguments given in the cond hash for the where clause

        @note this is equivalent to calling selectRows() with \c where = \c cond
    */
    @SuppressWarnings("unchecked")
    HashMap<String, Object>[] findAll() throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("findAll");
    }

    //! returns a descriptive string of the datasource (without the password) and the table name (with a possible qualifier for schema, etc)
    /** Used in exception descriptions

        @since SqlUtil 1.3
    */
    public String getDesc() throws Throwable {
        return (String)obj.callMethod("getDesc");
    }

    //! returns the base type of the underlying object (normally \"table\", some DB-specific implementations may support others like \c "view")
    public String getBaseType() throws Throwable {
        return (String)obj.callMethod("getBaseType");
    }

    //! returns the name of the table to be used in SQL (with a possible qualifier for schema, etc)
    public String getSqlName() throws Throwable {
        return (String)obj.callMethod("getSqlName");
    }

    //! returns the column name for use in SQL strings; subclasses can return a special string in case the column name is a reserved word
    public String getColumnSqlName(String col) throws Throwable {
        return (String)obj.callMethod("getColumnSqlName", col);
    }

    //! returns a list of column names for use in SQL strings; subclasses can process the argument list in case a column name is a reserved word
    String[] getColumnSqlNames(String[] cols) throws Throwable {
        return (String[])obj.callMethod("getColumnSqlNames", (Object)cols);
    }

    //! commits the current transaction on the underlying @ref Qore::SQL::AbstractDatasource
    public void commit() throws Throwable {
        obj.callMethod("commit");
    }

    //! rolls back the current transaction on the underlying @ref Qore::SQL::AbstractDatasource
    public void rollback() throws Throwable {
        obj.callMethod("rollback");
    }

    //! returns true if the table has been read from or created in the database, false if not
    /** @par Example:
        @code{.py}
boolean b = table.inDb();
        @endcode

        @return true if the table has been read from or created in the database, false if not

        @note this method only returns a flag if the object contains configuration already retrieved from the database, @see checkExistence() for a method that will check the database if the table exists in case the table is not already known to exist in the database
    */
    public boolean inDb() throws Throwable {
        return (boolean)obj.callMethod("inDb");
    }

    //! drops the table from the database without any transaction management
    /** @par Example:
        @code{.py}
table.drop();
        @endcode

        @param opt optional callback options; see @ref AbstractDatabase::CallbackOptions for more info

        @throw OPTION-ERROR invalid or unknown callback option

        @note Transaction management is normally not performed when dropping tables, however this method uses the Qore::SQL::AbstractDatasource::exec() method, which normally participates in acquiring a transaction lock for the underlying datasource object; therefore after this method executes normally the transaction lock will be dedicated to the calling thread.
     */
    public void drop(HashMap<String, Object> opt) throws Throwable {
        obj.callMethod("drop", opt);
    }

        //! drops the table from the database without any transaction management
    /** @par Example:
        @code{.py}
table.drop();
        @endcode

        @throw OPTION-ERROR invalid or unknown callback option

        @note Transaction management is normally not performed when dropping tables, however this method uses the Qore::SQL::AbstractDatasource::exec() method, which normally participates in acquiring a transaction lock for the underlying datasource object; therefore after this method executes normally the transaction lock will be dedicated to the calling thread.
     */
    public void drop() throws Throwable {
        obj.callMethod("drop");
    }

    //! executes some SQL with optional arguments so that if an error occurs the current transaction state is not lost
    /** @par Example:
        @code{.py}
t.tryExec("drop table tmp_table");
        @endcode

        Include any arguments in the parameter list after the \a sql argument

        @param sql the SQL to execute

        @return any return value from the SQL command executed
     */
    public Object tryExec(String sql) throws Throwable {
        return obj.callMethod("tryExec");
    }

    //! executes some SQL with optional arguments so that if an error occurs the current transaction state is not lost
    /** @par Example:
        @code{.py}
t.tryExec("delete from tmp_table where id = %v and name = %v", arglist);
        @endcode

        @param sql the SQL to execute
        @param args the bind / placeholder or other arguments corresponding to the SQL string

        @return any return value from the SQL command executed
     */
    public Object tryExecArgs(String sql, Object[] args) throws Throwable {
        return obj.callMethod("tryExecArgs", sql, (Object)args);
    }

    //! executes some SQL so that if an error occurs the current transaction state is not lost
    /** @par Example:
        @code{.py}
t.tryExecRaw("drop table tmp_table");
        @endcode

        Include any arguments in the parameter list after the \a sql argument

        @param sql the SQL to execute

        @return any return value from the SQL command executed
     */
    public Object tryExecRaw(String sql) throws Throwable {
        return obj.callMethod("tryExecRaw", sql);
    }

    //! returns the sql required to drop the table; reimplement in subclasses if necessary
    /** @par Example:
        @code{.py}
list l = table.getDropSql();
        @endcode

        @param opt optional callback options; see @ref AbstractDatabase::CallbackOptions for more info

        @return a list of strings that can be used to drop the table and any other objects assocatied with the table (for example: PostgreSQL table trigger function(s))

        @throw OPTION-ERROR invalid or unknown callback option
    */
    public String[] getDropSql(Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getDropSql", opt);
    }

    //! returns the sql required to drop the table; reimplement in subclasses if necessary
    /** @par Example:
        @code{.py}
list l = table.getDropSql();
        @endcode

        @return a list of strings that can be used to drop the table and any other objects assocatied with the table (for example: PostgreSQL table trigger function(s))

        @throw OPTION-ERROR invalid or unknown callback option
    */
    public String[] getDropSql() throws Throwable {
        return (String[])obj.callMethod("getDropSql");
    }

    //! truncates all the table data without any transaction management
    /** @par Example:
        @code{.py}
table.truncate();
        @endcode

        @note Transaction management may not be applied when truncating tables depending on the database driver (for example truncating tables in Oracle does not participate in transaction management), however this method uses the Qore::SQL::AbstractDatasource::exec() method, which normally participates in acquiring a transaction lock for the underlying datasource object; therefore after this method executes normally the transaction lock will be dedicated to the calling thread.
     */
    public void truncate() throws Throwable {
        obj.callMethod("truncate");
    }

    //! gets the SQL that can be used to truncate the table
    /** @par Example:
        @code{.py}
String sql = table.getTruncateSql();
        @endcode

        @param opt a hash of options for the SQL string; see @ref SqlUtil::AbstractTable.AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to truncate the table

        @throw OPTION-ERROR invalid or unsupported option passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getTruncateSql(Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getTruncateSql", opt);
    }

    //! gets the SQL that can be used to truncate the table
    /** @par Example:
        @code{.py}
String sql = table.getTruncateSql();
        @endcode

        @return the SQL that can be used to truncate the table

        @throw OPTION-ERROR invalid or unsupported option passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getTruncateSql() throws Throwable {
        return (String)obj.callMethod("getTruncateSql");
    }

    //! creates the table with all associated properties (indexes, constraints, etc) without any transaction management
    /** @par Example:
        @code{.py}
table.create();
        @endcode

        @param opt a hash of options for the SQL creation strings

        @note Transaction management is normally not performed when creating tables, however this method uses the Qore::SQL::AbstractDatasource::exec() method, which normally participates in acquiring a transaction lock for the underlying datasource object; therefore after this method executes normally the transaction lock will be dedicated to the calling thread.

        @throw CREATE-TABLE-ERROR table has already been read from or created in the database
     */
    public void create(Map<String, Object> opt) throws Throwable {
        obj.callMethod("create", opt);
    }

    //! creates the table with all associated properties (indexes, constraints, etc) without any transaction management
    /** @par Example:
        @code{.py}
table.create();
        @endcode

        @note Transaction management is normally not performed when creating tables, however this method uses the Qore::SQL::AbstractDatasource::exec() method, which normally participates in acquiring a transaction lock for the underlying datasource object; therefore after this method executes normally the transaction lock will be dedicated to the calling thread.

        @throw CREATE-TABLE-ERROR table has already been read from or created in the database
     */
    public void create() throws Throwable {
        obj.callMethod("create");
    }

    //! returns true if the table has no data rows, false if not
    /** @par Example:
        @code{.py}
boolean b = table.emptyData();
        @endcode

        @return true if the table has no data rows, false if not

        @see
        - empty()
        - rowCount()
    */
    public boolean emptyData() throws Throwable {
        return (boolean)obj.callMethod("emptyData");
    }

    //! returns true if the table has no definitions, false if not
    /** @par Example:
        @code{.py}
boolean b = table.empty();
        @endcode

        @return true if the table has no definitions, false if not

        @see emptyData()
    */
    public boolean empty() throws Throwable {
        return (boolean)obj.callMethod("empty");
    }

    //! creates the object from a table description hash
    /** @param desc a @ref table_desc_hash "table description hash" describing the table
        @param opt an optional hash of options for the table creation string; see @ref sqlutil.AbstractTableTableOptions for common options; each driver can support additional driver-specific options

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw DESCRIPTION-ERROR invalid or unsupported description hash value passed
    */
    public void setupTable(Map<String, Object> desc, Map<String, Object> opt) throws Throwable {
        obj.callMethod("setupTable", desc, opt);
    }

    //! creates the object from a table description hash
    /** @param desc a @ref table_desc_hash "table description hash" describing the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw DESCRIPTION-ERROR invalid or unsupported description hash value passed
    */
    public void setupTable(Map<String, Object> desc) throws Throwable {
        obj.callMethod("setupTable", desc);
    }

    //! adds a column to the table; if the table is already known to be in the database, then it is added in the database also immediately; otherwise it is only added internally and can be created when create() is called for example
    /** @par Example:
        @code{.java}
String sql;
table.addColumn("name", column_hash, false, sql);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the column
        @param opt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type string that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (False) or used as it is (True) to allow to use DBMS native functions or features. Defaults to False. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes
        @param nullable if @ref Qore::True "True" then the column can hold NULL values; note that primary key columns cannot be nullable

        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column already exists, invalid column data

        @note make sure and add a \c default_value value when adding a column with a \c "not null" constraint with existing data

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public void addColumn(String cname, Map<String, Object> opt, boolean nullable) throws Throwable {
        obj.callMethod("addColumn", cname, opt, nullable);
    }

    //! adds a nullable column to the table; if the table is already known to be in the database, then it is added in the database also immediately; otherwise it is only added internally and can be created when create() is called for example
    /** @par Example:
        @code{.java}
String sql;
table.addColumn("name", column_hash, false, sql);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the column
        @param opt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type string that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (False) or used as it is (True) to allow to use DBMS native functions or features. Defaults to False. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes

        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column already exists, invalid column data

        @note make sure and add a \c default_value value when adding a column with a \c "not null" constraint with existing data

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public void addColumn(String cname, Map<String, Object> opt) throws Throwable {
        obj.callMethod("addColumn", cname, opt);
    }

    //! returns a list of SQL strings that can be use to add a column to the table
    /** @par Example:
        @code{.py}
list l = table.getAddColumnSql("name", copt, false);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the column
        @param copt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type String that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (false) or used as it is (true) to allow to use DBMS native functions or features. Defaults to false. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes
        @param nullable if true then the column can hold NULL values; note that primary key columns cannot be nullable
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return a list of SQL strings that can be use to add a column to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column already exists, invalid column data

        @note
        - make sure and add a \c default_value value when adding a column with a \c "not null" constraint with existing data
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getAddColumnSql(String cname, Map<String, Object> copt, boolean nullable, Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getAddColumnSql", cname, copt, nullable, opt);
    }

    //! returns a list of SQL strings that can be use to add a column to the table
    /** @par Example:
        @code{.py}
list l = table.getAddColumnSql("name", copt, false);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the column
        @param copt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type String that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (false) or used as it is (true) to allow to use DBMS native functions or features. Defaults to false. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes
        @param nullable if true then the column can hold NULL values; note that primary key columns cannot be nullable

        @return a list of SQL strings that can be use to add a column to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column already exists, invalid column data

        @note
        - make sure and add a \c default_value value when adding a column with a \c "not null" constraint with existing data
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getAddColumnSql(String cname, Map<String, Object> copt, boolean nullable) throws Throwable {
        return (String[])obj.callMethod("getAddColumnSql", cname, copt, nullable);
    }

    //! returns a list of SQL strings that can be use to add a nullable column to the table
    /** @par Example:
        @code{.py}
String[] l = table.getAddColumnSql("name", copt);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the column
        @param copt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type String that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (false) or used as it is (true) to allow to use DBMS native functions or features. Defaults to false. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes

        @return a list of SQL strings that can be use to add a column to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column already exists, invalid column data

        @note
        - make sure and add a \c default_value value when adding a column with a \c "not null" constraint with existing data
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getAddColumnSql(String cname, Map<String, Object> copt) throws Throwable {
        return (String[])obj.callMethod("getAddColumnSql", cname, copt);
    }

    //! gets a list of SQL strings that can be used to modify an existing column in the table
    /** @par Example:
        @code{.py}
String[] l = table.getModifyColumnSql("name", copt, false);
        @endcode

        @param cname the name of the column
        @param copt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type String that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (false) or used as it is (true) to allow to use DBMS native functions or features. Defaults to false. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes
        @param nullable if true then the column can hold NULL values; note that primary key columns cannot be nullable
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return a list of SQL strings that can be used to modify an existing column in the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column does not exist, invalid column data

        @note
        - make sure and add a \c default_value value when modifying a column to have a \c "not null" constraint with existing data
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getModifyColumnSql(String cname, Map<String, Object> copt, boolean nullable, Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getModifyColumnSql", cname, copt, nullable, opt);
    }

    //! gets a list of SQL strings that can be used to modify an existing column in the table
    /** @par Example:
        @code{.py}
String[] l = table.getModifyColumnSql("name", copt, false);
        @endcode

        @param cname the name of the column
        @param copt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type String that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (false) or used as it is (true) to allow to use DBMS native functions or features. Defaults to false. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes
        @param nullable if true then the column can hold NULL values; note that primary key columns cannot be nullable

        @return a list of SQL strings that can be used to modify an existing column in the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column does not exist, invalid column data

        @note
        - make sure and add a \c default_value value when modifying a column to have a \c "not null" constraint with existing data
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getModifyColumnSql(String cname, Map<String, Object> copt, boolean nullable) throws Throwable {
        return (String[])obj.callMethod("getModifyColumnSql", cname, copt, nullable);
    }

    //! gets a list of SQL strings that can be used to modify an existing column in the table
    /** @par Example:
        @code{.py}
String[] l = table.getModifyColumnSql("name", copt, false);
        @endcode

        @param cname the name of the column
        @param copt a hash describing the column; the following keys are permitted (other column options may be supported depending on the underlying AbstractTable implementation):
        - \c qore_type: a qore type String that will be converted to a native DB type with some default conversion;
        - \c native_type: the native database column type; if both \c native_type and \c qore_type are given then \c native_type is used
        - \c size: for data types requiring a size component, the size; for numeric columns this represents the precision for example
        - \c scale: for numeric data types, this value gives the scale
        - \c default_value: the default value for the column
        - \c default_value_native: a boolean flag to say if a \c default_value should be validated against table column type (false) or used as it is (true) to allow to use DBMS native functions or features. Defaults to false. It is strongly recommended to use \c default_value_native for \c default_value in \c driver specific sub-hash to avoid non-portable schema hashes

        @return a list of SQL strings that can be used to modify an existing column in the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR no \c native_type or \c qore_type keys in column option hash, column does not exist, invalid column data

        @note
        - make sure and add a \c default_value value when modifying a column to have a \c "not null" constraint with existing data
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getModifyColumnSql(String cname, Map<String, Object> copt) throws Throwable {
        return (String[])obj.callMethod("getModifyColumnSql", cname, copt);
    }

    //! gets an SQL String that can be used to rename an existing column in the table
    /** @par Example:
        @code{.py}
String sql = table.getRenameColumnSql("name", "family_name");
        @endcode

        @param old_name the current name of the column
        @param new_name the new name of the column
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to rename an existing column in the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR if the old column does not exist in the table or the new column already does

        @see getModifyColumnSql() for a method that allows the column definition to be updated

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
    */
    public String getRenameColumnSql(String old_name, String new_name, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getRenameColumnSql", old_name, new_name, opt);
    }

    //! gets an SQL String that can be used to rename an existing column in the table
    /** @par Example:
        @code{.py}
String sql = table.getRenameColumnSql("name", "family_name");
        @endcode

        @param old_name the current name of the column
        @param new_name the new name of the column
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to rename an existing column in the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR if the old column does not exist in the table or the new column already does

        @see getModifyColumnSql() for a method that allows the column definition to be updated

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
    */
    public String getRenameColumnSql(String old_name, String new_name) throws Throwable {
        return (String)obj.callMethod("getRenameColumnSql", old_name, new_name);
    }

    //! returns the SQL that can be used to add a primary key to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddPrimaryKeySql("pk_mytable", "id", pkopt, opt);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param pkname the name of the new primary key constraint
        @param cols a single column name or a list of columns that make up the primary key
        @param pkopt a hash of options for the new primary key; each driver may implement its own options; for common options, see @ref org.qore.sqlutil.AbstractTableConstraintOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to add a primary key to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw PRIMARY-KEY-ERROR the table already has a primary key or invalid columns or options passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddPrimaryKeySql(String pkname, String[] cols, Map<String, Object> pkopt, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getAddPrimaryKeySql", pkname, cols, pkopt, opt);
    }

    //! returns the SQL that can be used to add a primary key to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddPrimaryKeySql("pk_mytable", "id", pkopt);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param pkname the name of the new primary key constraint
        @param cols a single column name or a list of columns that make up the primary key
        @param pkopt a hash of options for the new primary key; each driver may implement its own options; for common options, see @ref ConstraintOptions

        @return the SQL that can be used to add a primary key to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw PRIMARY-KEY-ERROR the table already has a primary key or invalid columns or options passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddPrimaryKeySql(String pkname, String[] cols, Map<String, Object> pkopt) throws Throwable {
        return (String)obj.callMethod("getAddPrimaryKeySql", pkname, cols, pkopt);
    }

    //! returns the SQL that can be used to add a primary key to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddPrimaryKeySql("pk_mytable", "id");
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param pkname the name of the new primary key constraint
        @param cols a single column name or a list of columns that make up the primary key

        @return the SQL that can be used to add a primary key to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw PRIMARY-KEY-ERROR the table already has a primary key or invalid columns or options passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddPrimaryKeySql(String pkname, String[] cols) throws Throwable {
        return (String)obj.callMethod("getAddPrimaryKeySql", pkname, cols);
    }

    //! gets a list of SQL strings to drop all constraints and indexes with the given column name; if the column does not exist then an empty list is returned
    /** @par Example:
        @code{.py}
String[] strlist = table.getDropAllConstraintsAndIndexesOnColumnSql("status");
        @endcode

        @param cname the name of the column
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @throw OPTION-ERROR invalid or unsupported option passed

        @note
        - this method retrieves current primary key definition from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
    */
    public String[] getDropAllConstraintsAndIndexesOnColumnSql(String cname, Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getDropAllConstraintsAndIndexesOnColumnSql", cname, opt);
    }

    //! gets a list of SQL strings to drop all constraints and indexes with the given column name; if the column does not exist then an empty list is returned
    /** @par Example:
        @code{.py}
String[] strlist = table.getDropAllConstraintsAndIndexesOnColumnSql("status");
        @endcode

        @param cname the name of the column

        @throw OPTION-ERROR invalid or unsupported option passed

        @note
        - this method retrieves current primary key definition from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
    */
    public String[] getDropAllConstraintsAndIndexesOnColumnSql(String cname) throws Throwable {
        return (String[])obj.callMethod("getDropAllConstraintsAndIndexesOnColumnSql", cname);
    }

    //! gets a list of SQL strings that can be used to drop the primary key from the table
    /** @par Example:
        @code{.py}
String[] l = table.getDropPrimaryKeySql();
        @endcode

        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return a list of SQL strings that can be used to drop the primary key from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw PRIMARY-KEY-ERROR the table has no primary key

        @note
        - this method retrieves current primary key definition from the database if not already loaded or defined
        - if there are known foreign contraints on the primary key, SQL for dropping those constraints is also returned
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getDropPrimaryKeySql(Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getDropPrimaryKeySql", opt);
    }

    //! gets a list of SQL strings that can be used to drop the primary key from the table
    /** @par Example:
        @code{.py}
String[] l = table.getDropPrimaryKeySql();
        @endcode

        @return a list of SQL strings that can be used to drop the primary key from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw PRIMARY-KEY-ERROR the table has no primary key

        @note
        - this method retrieves current primary key definition from the database if not already loaded or defined
        - if there are known foreign contraints on the primary key, SQL for dropping those constraints is also returned
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getDropPrimaryKeySql() throws Throwable {
        return (String[])obj.callMethod("getDropPrimaryKeySql");
    }

    //! returns an SQL String that can be used to add a unique constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddUniqueConstraintSql("uk_mytable", "name", ukopt);
        @endcode

        @param cname the name of the new unique constraint
        @param cols a single column name or a list of columns that make up the unique constraint
        @param ukopt a hash of options for the new unique constraint; each driver may implement its own options; for common options, see @ref ConstraintOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to add a unique constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw UNIQUE-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddUniqueConstraintSql(String cname, String[] cols, Map<String, Object> ukopt, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getAddUniqueConstraintSql", cname, cols, ukopt, opt);
    }

    //! returns an SQL String that can be used to add a unique constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddUniqueConstraintSql("name", ukopt);
        @endcode

        @param cname the name of the new unique constraint
        @param cols a single column name or a list of columns that make up the unique constraint
        @param ukopt a hash of options for the new unique constraint; each driver may implement its own options; for common options, see @ref ConstraintOptions

        @return an SQL String that can be used to add a unique constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw UNIQUE-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddUniqueConstraintSql(String cname, String[] cols, Map<String, Object> ukopt) throws Throwable {
        return (String)obj.callMethod("getAddUniqueConstraintSql", cname, cols, ukopt);
    }

    //! returns an SQL String that can be used to add a unique constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddUniqueConstraintSql("name", cols);
        @endcode

        @param cname the name of the new unique constraint
        @param cols a single column name or a list of columns that make up the unique constraint

        @return an SQL String that can be used to add a unique constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw UNIQUE-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddUniqueConstraintSql(String cname, String[] cols) throws Throwable {
        return (String)obj.callMethod("getAddUniqueConstraintSql", cname, cols);
    }

    //! returns an SQL String that can be used to add an index to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddIndexSql("uk_mytable_name", true, "name", ixopt);
        @endcode

        @param iname the name of the new index
        @param unique a flag to tell if the new index should be unique or not
        @param cols a single column name or a list of columns that make up the index
        @param ixopt a hash of options for the new index; each driver may implement its own options; for common options, see @ref IndexOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to add an index to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw INDEX-ERROR the table already has an index with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddIndexSql(String iname, boolean unique, String[] cols, Map<String, Object> ixopt, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getAddIndexSql", iname, unique, cols, ixopt, opt);
    }

    //! returns an SQL String that can be used to add an index to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddIndexSql("uk_mytable_name", true, "name", ixopt);
        @endcode

        @param iname the name of the new index
        @param unique a flag to tell if the new index should be unique or not
        @param cols a single column name or a list of columns that make up the index
        @param ixopt a hash of options for the new index; each driver may implement its own options; for common options, see @ref IndexOptions

        @return an SQL String that can be used to add an index to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw INDEX-ERROR the table already has an index with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddIndexSql(String iname, boolean unique, String[] cols, Map<String, Object> ixopt) throws Throwable {
        return (String)obj.callMethod("getAddIndexSql", iname, unique, cols, ixopt);
    }

    //! returns an SQL String that can be used to add an index to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddIndexSql("uk_mytable_name", true, "name", ixopt);
        @endcode

        @param iname the name of the new index
        @param unique a flag to tell if the new index should be unique or not
        @param cols a single column name or a list of columns that make up the index

        @return an SQL String that can be used to add an index to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw INDEX-ERROR the table already has an index with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddIndexSql(String iname, boolean unique, String[] cols) throws Throwable {
        return (String)obj.callMethod("getAddIndexSql", iname, unique, cols);
    }

    //! gets the SQL that can be used to drop an index from the table
    /** @par Example:
        @code{.py}
String sql = table.getDropIndexSql("uk_mytable_name");
        @endcode

        @param iname the name of the index to drop
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to drop an index from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw INDEX-ERROR the given index does not exist in the table

        @note
        - this method retrieves current index definitions from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String getDropIndexSql(String iname, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getDropIndexSql", iname, opt);
    }

    //! gets the SQL that can be used to drop an index from the table
    /** @par Example:
        @code{.py}
String sql = table.getDropIndexSql("uk_mytable_name");
        @endcode

        @param iname the name of the index to drop

        @return the SQL that can be used to drop an index from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw INDEX-ERROR the given index does not exist in the table

        @note
        - this method retrieves current index definitions from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String getDropIndexSql(String iname) throws Throwable {
        return (String)obj.callMethod("getDropIndexSql", iname);
    }

    //! returns an SQL String that can be used to add a foreign constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddForeignConstraintSql("fk_mytable_other_table", ("name", "version"), "other_table");
        @endcode

        @param cname the name of the new foreign constraint
        @param cols a single column name or a list of columns in the local table that make up the foreign constraint
        @param table the name of the other table that the constraint targets
        @param tcols a single column name or a list of columns in the foreign table or null meaning that the column names are the same as in the local table; if column names are given the same number of columns must be given in the local and foreign tables
        @param fkopt a hash of options for the new foreign constraint; each driver may implement its own options; for common options, see @ref ForeignConstraintOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to add a foreign constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw FOREIGN-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddForeignConstraintSql(String cname, String[] cols, String table, String[] tcols, Map<String, Object> fkopt, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getAddForeignConstraintSql", cname, cols, table, tcols, fkopt, opt);
    }

    //! returns an SQL String that can be used to add a foreign constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddForeignConstraintSql("fk_mytable_other_table", cols, "other_table");
        @endcode

        @param cname the name of the new foreign constraint
        @param cols a single column name or a list of columns in the local table that make up the foreign constraint
        @param table the name of the other table that the constraint targets
        @param tcols a single column name or a list of columns in the foreign table or null meaning that the column names are the same as in the local table; if column names are given the same number of columns must be given in the local and foreign tables
        @param fkopt a hash of options for the new foreign constraint; each driver may implement its own options; for common options, see @ref ForeignConstraintOptions

        @return an SQL String that can be used to add a foreign constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw FOREIGN-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddForeignConstraintSql(String cname, String[] cols, String table, String[] tcols, Map<String, Object> fkopt) throws Throwable {
        return (String)obj.callMethod("getAddForeignConstraintSql", cname, cols, table, tcols, fkopt);
    }

    //! returns an SQL String that can be used to add a foreign constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddForeignConstraintSql("fk_mytable_other_table", cols, "other_table");
        @endcode

        @param cname the name of the new foreign constraint
        @param cols a single column name or a list of columns in the local table that make up the foreign constraint
        @param table the name of the other table that the constraint targets
        @param tcols a single column name or a list of columns in the foreign table or null meaning that the column names are the same as in the local table; if column names are given the same number of columns must be given in the local and foreign tables

        @return an SQL String that can be used to add a foreign constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw FOREIGN-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddForeignConstraintSql(String cname, String[] cols, String table, String[] tcols) throws Throwable {
        return (String)obj.callMethod("getAddForeignConstraintSql", cname, cols, table, tcols);
    }

    //! returns an SQL String that can be used to add a foreign constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddForeignConstraintSql("fk_mytable_other_table", cols, "other_table");
        @endcode

        @param cname the name of the new foreign constraint
        @param cols a single column name or a list of columns in the local table that make up the foreign constraint
        @param table the name of the other table that the constraint targets

        @return an SQL String that can be used to add a foreign constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw FOREIGN-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddForeignConstraintSql(String cname, String[] cols, String table) throws Throwable {
        return (String)obj.callMethod("getAddForeignConstraintSql", cname, cols, table);
    }

    //! returns an SQL String that can be used to add a check constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddCheckConstraintSql("check_mytable_id", "id > 10");
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the new constraint
        @param src the source of the constraint clause
        @param copt a hash of options for the new constraint; each driver may implement its own options; for common options, see @ref ConstraintOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to add a check constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw CHECK-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddCheckConstraintSql(String cname, String src, Map<String, Object> copt, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getAddForeignConstraintSql", cname, src, copt, opt);
    }

    //! returns an SQL String that can be used to add a check constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddCheckConstraintSql("check_mytable_id", "id > 10");
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the new constraint
        @param src the source of the constraint clause
        @param copt a hash of options for the new constraint; each driver may implement its own options; for common options, see @ref ConstraintOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to add a check constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw CHECK-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddCheckConstraintSql(String cname, String src, Map<String, Object> copt) throws Throwable {
        return (String)obj.callMethod("getAddCheckConstraintSql", cname, src, copt);
    }

    //! returns an SQL String that can be used to add a check constraint to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddCheckConstraintSql("check_mytable_id", "id > 10");
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param cname the name of the new constraint
        @param src the source of the constraint clause
        @param copt a hash of options for the new constraint; each driver may implement its own options; for common options, see @ref ConstraintOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return an SQL String that can be used to add a check constraint to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw CHECK-CONSTRAINT-ERROR the table already has a constraint with the given name or invalid columns or options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String getAddCheckConstraintSql(String cname, String src) throws Throwable {
        return (String)obj.callMethod("getAddCheckConstraintSql", cname, src);
    }

    //! gets the SQL that can be used to drop a constraint from the table; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint
    /** @par Example:
        @code{.py}
String sql = table.getDropConstraintSql("uk_mytable_name");
        @endcode

        @param cname the name of the constraint to drop
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to drop a constraint from the table; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw CONSTRAINT-ERROR the given constraint does not exist in the table

        @note
        - this method retrieves current constraint definitions from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String getDropConstraintSql(String cname, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getDropConstraintSql", cname, opt);
    }

    //! gets the SQL that can be used to drop a constraint from the table; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint
    /** @par Example:
        @code{.py}
String sql = table.getDropConstraintSql("uk_mytable_name");
        @endcode

        @param cname the name of the constraint to drop
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to drop a constraint from the table; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw CONSTRAINT-ERROR the given constraint does not exist in the table

        @note
        - this method retrieves current constraint definitions from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String getDropConstraintSql(String cname) throws Throwable {
        return (String)obj.callMethod("getDropConstraintSql", cname);
    }

    //! gets the SQL that can be used to drop a constraint from the table if it exists, otherwise returns null; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint
    /** @par Example:
        @code{.py}
*String sql = table.getDropConstraintIfExistsSql("uk_mytable_name");
        @endcode

        @param cname the name of the constraint to drop
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to drop a constraint from the table if it exists, otherwise returns null; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint

        @throw OPTION-ERROR invalid or unsupported option passed

        @note
        - this method retrieves current constraint definitions from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String getDropConstraintIfExistsSql(String cname, Map<String, Object> opt) throws Throwable {
        return (String)obj.callMethod("getDropConstraintIfExistsSql", cname, opt);
    }

    //! gets the SQL that can be used to drop a constraint from the table if it exists, otherwise returns null; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint
    /** @par Example:
        @code{.py}
*String sql = table.getDropConstraintIfExistsSql("uk_mytable_name");
        @endcode

        @param cname the name of the constraint to drop

        @return the SQL that can be used to drop a constraint from the table if it exists, otherwise returns null; this can be any constraint on the table, a primary key, a foreign key constraint, or a generic constraint

        @throw OPTION-ERROR invalid or unsupported option passed

        @note
        - this method retrieves current constraint definitions from the database if not already loaded or defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String getDropConstraintIfExistsSql(String cname) throws Throwable {
        return (String)obj.callMethod("getDropConstraintIfExistsSql", cname);
    }

    //! returns a list of SQL strings that can be used to add a trigger to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddTriggerSql("trig_mytable", trigger_src);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param tname the name of the new trigger
        @param src the source of the trigger
        @param topt a hash of options for the new trigger; each driver may implement its own options; for common options, see @ref TriggerOptions
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return a list of SQL strings that can be used to add a trigger to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw TRIGGER-ERROR the table already has a trigger with the given name or invalid options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String[] getAddTriggerSql(String tname, String src, Map<String, Object> topt, Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getAddTriggerSql", tname, src, topt, opt);
    }

    //! returns a list of SQL strings that can be used to add a trigger to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddTriggerSql("trig_mytable", trigger_src);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param tname the name of the new trigger
        @param src the source of the trigger
        @param topt a hash of options for the new trigger; each driver may implement its own options; for common options, see @ref TriggerOptions

        @return a list of SQL strings that can be used to add a trigger to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw TRIGGER-ERROR the table already has a trigger with the given name or invalid options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String[] getAddTriggerSql(String tname, String src, Map<String, Object> topt) throws Throwable {
        return (String[])obj.callMethod("getAddTriggerSql", tname, src, topt);
    }

    //! returns a list of SQL strings that can be used to add a trigger to the table
    /** @par Example:
        @code{.py}
String sql = table.getAddTriggerSql("trig_mytable", trigger_src);
        @endcode

        In case the table is already in the database, this method commits the transaction on success and rolls back the transaction if there's an error.

        @param tname the name of the new trigger
        @param src the source of the trigger

        @return a list of SQL strings that can be used to add a trigger to the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw TRIGGER-ERROR the table already has a trigger with the given name or invalid options were passed

        @see inDb() for a method that tells if the table is already in the database or not

        @note if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)
     */
    public String[] getAddTriggerSql(String tname, String src) throws Throwable {
        return (String[])obj.callMethod("getAddTriggerSql", tname, src);
    }

    //! returns SQL that can be used to drop the given trigger from the table
    /** @par Example:
        @code{.py}
String sql = table.getDropTriggerSql("trig_mytable");
        @endcode

        @param tname the name of the trigger to drop
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to drop the given trigger from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw TRIGGER-ERROR the given trigger does not exist

        @note
        - this method retrieves all current trigger definitions from the database if none have already been defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getDropTriggerSql(String tname, Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getDropTriggerSql", tname, opt);
    }

    //! returns SQL that can be used to drop the given trigger from the table
    /** @par Example:
        @code{.py}
String sql = table.getDropTriggerSql("trig_mytable");
        @endcode

        @param tname the name of the trigger to drop

        @return the SQL that can be used to drop the given trigger from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw TRIGGER-ERROR the given trigger does not exist

        @note
        - this method retrieves all current trigger definitions from the database if none have already been defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getDropTriggerSql(String tname) throws Throwable {
        return (String[])obj.callMethod("getDropTriggerSql", tname);
    }

    //! returns the SQL that can be used to drop a column from the table
    /** @par Example:
        @code{.py}
String sql = table.getDropColumnSql("notes_2");
        @endcode

        @param cname the name of the column to drop
        @param opt a hash of options for the SQL string; see @ref AlignTableOptions for common options; each driver can support additional driver-specific options

        @return the SQL that can be used to drop a column from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR the named column is not present in the table

        @note
        - this method retrieves the column definitions from the database if none have already been defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getDropColumnSql(String cname, Map<String, Object> opt) throws Throwable {
        return (String[])obj.callMethod("getDropColumnSql", cname, opt);
    }

    //! returns the SQL that can be used to drop a column from the table
    /** @par Example:
        @code{.py}
String sql = table.getDropColumnSql("notes_2");
        @endcode

        @param cname the name of the column to drop

        @return the SQL that can be used to drop a column from the table

        @throw OPTION-ERROR invalid or unsupported option passed
        @throw COLUMN-ERROR the named column is not present in the table

        @note
        - this method retrieves the column definitions from the database if none have already been defined
        - if the @ref sql_callback_executed "sql_callback_executed option" is true in \a opt, then the changes are also effected in the current object, if not, then they are not (see @ref sql_callback_executed for more information)

        @see inDb() for a method that tells if the table is already in the database or not
     */
    public String[] getDropColumnSql(String cname) throws Throwable {
        return (String[])obj.callMethod("getDropColumnSql", cname);
    }

    //! inserts a row into the table without any transaction management; a transaction will be in progress after this method is successfully executed
    /** @ingroup inserts
        @par Example:
        @code{.py}
table.insert(row);
        @endcode

        @param row a hash representing the row to insert; hash values can also be set with @ref sql_iop_funcs to insert values based on SQL operations to be used directly in the insert statement

        @return in case the \c "returning" @ref InsertOptions "insert option" is used, a hash of return values is returned, otherwise null is returned

        @throw COLUMN-ERROR an unknown column was referenced in the hash to be inserted
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> insert(Map<String, Object> row) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("insert", row);
    }

    //! @ref insert() variant
    /**
        @param row a hash representing the row to insert; hash values can also be set with @ref sql_iop_funcs to insert values based on SQL operations to be used directly in the insert statement
        @param opt optional insert options; see @ref InsertOptions for more info
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> insert(Map<String, Object> row, Map<String, Object> opt) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("insert", row, opt);
    }

    //! returns true if the current database driver supports the \c "returning" clause in insert statements, false if not
    /** @return true if the current database driver supports the \c "returning" clause in insert statements, false if not
        @since %SqlUtil 1.3
    */
    public boolean hasReturning() throws Throwable {
        return (boolean)obj.callMethod("hasReturning");
    }

    //! inserts rows into a table based on a select statement from another table (which must be using the same datasource as the current table); a transaction will be in progress after this method is successfully executed
    /** @par Example:
        @code{.py}
int rows = table.insertFromSelect(("id", "name", "created"), source_table, (("columns": ("id", "name", "created"), "where": ("type": "CUSTOMER"))));
        @endcode

        @param cols the list of column names to use to insert in the current table
        @param source the source table for the select statement
        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param sql an optional reference to a String to return the SQL generated for the select statement
        @param opt optional SQL data operation callback options; see @ref SqlDataCallbackOptions for more inf

        @return the number of rows inserted

        @throw OPTION-ERROR invalid or unsupported option
        @throw COLUMN-ERROR unknown or invalid column in insert list
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note this method does not take insert options because it is executed entirely in the database server; use insertFromIterator() or insertFromIteratorCommit() to insert arbitrary data with insert options
    */
    public int insertFromSelect(String[] cols, AbstractTable source, Map<String, Object> sh, Map<String, Object> opt) throws Throwable {
        return (int)obj.callMethod("insertFromSelect", cols, source, sh, opt);
    }

    //! @ref insertFromSelectCommit() variant
    public int insertFromSelect(String[] cols, AbstractTable source, Map<String, Object> sh) throws Throwable {
        return (int)obj.callMethod("insertFromSelect", cols, source, sh);
    }

    //! @ref insertFromSelectCommit() variant
    public int insertFromSelect(String[] cols, AbstractTable source) throws Throwable {
        return (int)obj.callMethod("insertFromSelect", cols, source);
    }

    //! update or insert the data in the table according to the hash argument; the table must have a unique key to do this
    /** @par Example:
        @code{.py}
table.upsert(row);
        @endcode

        @param row a hash representing the row to insert or update
        @param upsert_strategy see @ref upsert_options for possible values for the upsert strategy
        @param opt a hash of options for the upsert operation; see @ref SqlUtil::AbstractTable::UpsertOptions for common options; each driver can support additional driver-specific options

        @return an integer code giving the result of the update; see @ref upsert_results for more information

        @throw COLUMN-ERROR an unknown column was referenced in the hash to be inserted
        @throw UPSERT-ERROR no primary key, unique constraint, or unique index for upsert; not all columns of the unique constraint/index are used in the upsert statement

        @note if upserting multiple rows; it's better to use getBulkUpsertClosure(), getUpsertClosure(), or getUpsertClosureWithValidation() and execute the closure on each row; when using this method, the overhead for setting up the upsert is made for each row which is very inefficient
     */
    public int upsert(Map<String, Object> row, int upsert_strategy, Map<String, Object> opt) throws Throwable {
        return (int)obj.callMethod("upsert", row, upsert_strategy, opt);
    }

    //! update or insert the data in the table according to the hash argument; the table must have a unique key to do this
    /** @par Example:
        @code{.py}
table.upsert(row);
        @endcode

        @param row a hash representing the row to insert or update
        @param upsert_strategy see @ref upsert_options for possible values for the upsert strategy

        @return an integer code giving the result of the update; see @ref upsert_results for more information

        @throw COLUMN-ERROR an unknown column was referenced in the hash to be inserted
        @throw UPSERT-ERROR no primary key, unique constraint, or unique index for upsert; not all columns of the unique constraint/index are used in the upsert statement

        @note if upserting multiple rows; it's better to use getBulkUpsertClosure(), getUpsertClosure(), or getUpsertClosureWithValidation() and execute the closure on each row; when using this method, the overhead for setting up the upsert is made for each row which is very inefficient
     */
    public int upsert(Map<String, Object> row, int upsert_strategy) throws Throwable {
        return (int)obj.callMethod("upsert", row, upsert_strategy);
    }

    //! this method upserts or merges data from the given foreign table and @ref select_option_hash "select option hash" into the current table; no transaction management is performed with this method
    /** @par Example:
        @code{.py}
on_success { table.commit(); table2.commit(); }
on_error { table.rollback(); table2.rollback(); }
hash h = table.upsertFromSelect(table2, ("where": ("account_type": "CUSTOMER")), AbstractTable::UpsertUpdateFirst);
        @endcode

        The table argument does not need to be in the same database as the current table; it can also
        be in a different database server or a database server of a different type (you can use this method to upsert or
        merge data to or from any database supported by SqlUtil).

        @param t the table for the source data; this does not need to be in the same database as the target (the current table), nor does it need to be the same database type
        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param upsert_strategy see @ref upsert_options for possible values for the upsert strategy
        @param opt a hash of options for the upsert operation; see @ref SqlUtil::AbstractTable::UpsertOptions for common options; each driver can support additional driver-specific options; note that this method ignores any \c "commit_block" option

        @return null if no actions were taken or a hash with the following keys assigned to numeric values indicating the number of rows processed (keys correspond to @ref UpsertResultDescriptionMap keys):
        - \c "inserted": the number of rows inserted
        - \c "verified": the number of rows updated unconditionally; note that this key is returned with all upsert strategy codes other than @ref UpsertSelectFirst instead of \c "updated"
        - \c "updated": the number of rows updated; note that this key is only returned if \a upsert_strategy is @ref UpsertSelectFirst, otherwise updated rows are reported as \c "verified" since rows are updated unconditionally with other the upsert strategy codes
        - \c "unchanged": the number of rows unchanged; this key can only be returned if \a upsert_strategy is @ref UpsertSelectFirst, @ref UpsertInsertOnly, or @ref UpsertUpdateOnly
        - \c "deleted": the number of rows deleted; this can only be returned if @ref SqlUtil::AbstractTable::UpsertOptions "upsert option" \c delete_others is true

        @throw OPTION-ERROR invalid or unsupported option
        @throw COLUMN-ERROR an unknown column was referenced in the hash to be inserted
        @throw UPSERT-ERROR no primary key, unique constraint, or unique index for upsert; not all columns of the unique constraint/index are used in the upsert statement

        @note
        - if @ref SqlUtil::AbstractTable::UpsertOptions "upsert option" \c delete_others is true, then a hash of primary key values in the input data is built as the input data is iterated.  After iterating, if the row count of the table and the input data matches, then nothing more is done, otherwise, every row of the table is iterated and compared to the primary key hash; if a row does not match a primary key value, then it is deleted.  This operation is only executed if \c delete_others is true and is expensive for large data sets.
       - this method uses an @ref Qore::SQL::AbstractSQLStatement "AbstractSQLStatement" object to pipeline the select data to the upsert code; to release the transaction lock acquired by the @ref Qore::SQL::AbstractSQLStatement "AbstractSQLStatement" object, a commit() or rollback() action must be executed on the underlying datasource object as in the example above
        - unlike insertFromSelect() and insertFromSelectCommit(), this method processes arbitrary input data and accepts @ref SqlUtil::AbstractTable::UpsertOptions "upsert options"

        @see
        - upsertFromSelect()
        - upsert()
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> upsertFromSelect(AbstractTable t, Map<String, Object> sh, int upsert_strategy, Map<String, Object> opt) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("upsertFromSelect", t, sh, upsert_strategy, opt);
    }

    //! this method upserts or merges data from the given foreign table and @ref select_option_hash "select option hash" into the current table; no transaction management is performed with this method
    /** @par Example:
        @code{.py}
on_success { table.commit(); table2.commit(); }
on_error { table.rollback(); table2.rollback(); }
hash h = table.upsertFromSelect(table2, ("where": ("account_type": "CUSTOMER")), AbstractTable::UpsertUpdateFirst);
        @endcode

        The table argument does not need to be in the same database as the current table; it can also
        be in a different database server or a database server of a different type (you can use this method to upsert or
        merge data to or from any database supported by SqlUtil).

        @param t the table for the source data; this does not need to be in the same database as the target (the current table), nor does it need to be the same database type
        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param upsert_strategy see @ref upsert_options for possible values for the upsert strategy

        @return null if no actions were taken or a hash with the following keys assigned to numeric values indicating the number of rows processed (keys correspond to @ref UpsertResultDescriptionMap keys):
        - \c "inserted": the number of rows inserted
        - \c "verified": the number of rows updated unconditionally; note that this key is returned with all upsert strategy codes other than @ref UpsertSelectFirst instead of \c "updated"
        - \c "updated": the number of rows updated; note that this key is only returned if \a upsert_strategy is @ref UpsertSelectFirst, otherwise updated rows are reported as \c "verified" since rows are updated unconditionally with other the upsert strategy codes
        - \c "unchanged": the number of rows unchanged; this key can only be returned if \a upsert_strategy is @ref UpsertSelectFirst, @ref UpsertInsertOnly, or @ref UpsertUpdateOnly
        - \c "deleted": the number of rows deleted; this can only be returned if @ref SqlUtil::AbstractTable::UpsertOptions "upsert option" \c delete_others is true

        @throw OPTION-ERROR invalid or unsupported option
        @throw COLUMN-ERROR an unknown column was referenced in the hash to be inserted
        @throw UPSERT-ERROR no primary key, unique constraint, or unique index for upsert; not all columns of the unique constraint/index are used in the upsert statement

        @note
        - if @ref SqlUtil::AbstractTable::UpsertOptions "upsert option" \c delete_others is true, then a hash of primary key values in the input data is built as the input data is iterated.  After iterating, if the row count of the table and the input data matches, then nothing more is done, otherwise, every row of the table is iterated and compared to the primary key hash; if a row does not match a primary key value, then it is deleted.  This operation is only executed if \c delete_others is true and is expensive for large data sets.
       - this method uses an @ref Qore::SQL::AbstractSQLStatement "AbstractSQLStatement" object to pipeline the select data to the upsert code; to release the transaction lock acquired by the @ref Qore::SQL::AbstractSQLStatement "AbstractSQLStatement" object, a commit() or rollback() action must be executed on the underlying datasource object as in the example above
        - unlike insertFromSelect() and insertFromSelectCommit(), this method processes arbitrary input data and accepts @ref SqlUtil::AbstractTable::UpsertOptions "upsert options"

        @see
        - upsertFromSelect()
        - upsert()
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> upsertFromSelect(AbstractTable t, Map<String, Object> sh, int upsert_strategy) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("upsertFromSelect", t, sh, upsert_strategy);
    }

    //! this method upserts or merges data from the given foreign table and @ref select_option_hash "select option hash" into the current table; no transaction management is performed with this method
    /** @par Example:
        @code{.py}
on_success { table.commit(); table2.commit(); }
on_error { table.rollback(); table2.rollback(); }
hash h = table.upsertFromSelect(table2, ("where": ("account_type": "CUSTOMER")), AbstractTable::UpsertUpdateFirst);
        @endcode

        The table argument does not need to be in the same database as the current table; it can also
        be in a different database server or a database server of a different type (you can use this method to upsert or
        merge data to or from any database supported by SqlUtil).

        @param t the table for the source data; this does not need to be in the same database as the target (the current table), nor does it need to be the same database type
        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument

        @return null if no actions were taken or a hash with the following keys assigned to numeric values indicating the number of rows processed (keys correspond to @ref UpsertResultDescriptionMap keys):
        - \c "inserted": the number of rows inserted
        - \c "verified": the number of rows updated unconditionally; note that this key is returned with all upsert strategy codes other than @ref UpsertSelectFirst instead of \c "updated"
        - \c "updated": the number of rows updated; note that this key is only returned if \a upsert_strategy is @ref UpsertSelectFirst, otherwise updated rows are reported as \c "verified" since rows are updated unconditionally with other the upsert strategy codes
        - \c "unchanged": the number of rows unchanged; this key can only be returned if \a upsert_strategy is @ref UpsertSelectFirst, @ref UpsertInsertOnly, or @ref UpsertUpdateOnly
        - \c "deleted": the number of rows deleted; this can only be returned if @ref SqlUtil::AbstractTable::UpsertOptions "upsert option" \c delete_others is true

        @throw OPTION-ERROR invalid or unsupported option
        @throw COLUMN-ERROR an unknown column was referenced in the hash to be inserted
        @throw UPSERT-ERROR no primary key, unique constraint, or unique index for upsert; not all columns of the unique constraint/index are used in the upsert statement

        @note
        - if @ref SqlUtil::AbstractTable::UpsertOptions "upsert option" \c delete_others is true, then a hash of primary key values in the input data is built as the input data is iterated.  After iterating, if the row count of the table and the input data matches, then nothing more is done, otherwise, every row of the table is iterated and compared to the primary key hash; if a row does not match a primary key value, then it is deleted.  This operation is only executed if \c delete_others is true and is expensive for large data sets.
       - this method uses an @ref Qore::SQL::AbstractSQLStatement "AbstractSQLStatement" object to pipeline the select data to the upsert code; to release the transaction lock acquired by the @ref Qore::SQL::AbstractSQLStatement "AbstractSQLStatement" object, a commit() or rollback() action must be executed on the underlying datasource object as in the example above
        - unlike insertFromSelect() and insertFromSelectCommit(), this method processes arbitrary input data and accepts @ref SqlUtil::AbstractTable::UpsertOptions "upsert options"

        @see
        - upsertFromSelect()
        - upsert()
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> upsertFromSelect(AbstractTable t, Map<String, Object> sh) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("upsertFromSelect", t, sh);
    }

    //! returns the number of rows in the table
    /** @par Example:
        @code{.py}
int cnt = table.rowCount();
        @endcode

        @return the number of rows in the table

        @note to see if the table is empty or not, use emptyData() as this is much faster than rowCount()

        @see emptyData()
     */
    public int rowCount() throws Throwable {
        return (int)obj.callMethod("rowCount");
    }

    //! returns an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments
    /** @par Example:
        @code{.py}
AbstractSQLStatement i = table.getStatement(sh, opts);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param opt optional SQL data operation callback options; see @ref SqlDataCallbackOptions for more info

        @return an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - the @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object created by a successful call to this method acquires a thread resource for the underlying @ref org.qore.AbstractDatasource "AbstractDatasource" object that must be released by calling @ref org.qore.AbstractDatasource::commit() "commit()" or @ref org.qore.AbstractDatasource::rollback() "rollback()", even if the statement does not acquire any database locks

        @see getStatementNoExec()

        @since %SqlUtil 1.5
     */
    public AbstractSQLStatement getStatement(Map<String, Object> sh, Map<String, Object> opt) throws Throwable {
        return new AbstractSQLStatement((QoreObject)obj.callMethodSave("getStatement", sh, opt));
    }

    //! returns an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments
    /** @par Example:
        @code{.py}
AbstractSQLStatement i = table.getStatement(sh);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument

        @return an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments

        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - the @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object created by a successful call to this method acquires a thread resource for the underlying @ref org.qore.AbstractDatasource "AbstractDatasource" object that must be released by calling @ref org.qore.AbstractDatasource::commit() "commit()" or @ref org.qore.AbstractDatasource::rollback() "rollback()", even if the statement does not acquire any database locks

        @see getStatementNoExec()

        @since %SqlUtil 1.5
     */
    public AbstractSQLStatement getStatement(Map<String, Object> sh) throws Throwable {
        return new AbstractSQLStatement((QoreObject)obj.callMethodSave("getStatement", sh));
    }

    //! returns an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate all the rows in the table
    /** @par Example:
        @code{.py}
AbstractSQLStatement i = table.getStatement();
        @endcode

        @return an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate all the rows in the table

        @note
        - the @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object created by a successful call to this method acquires a thread resource for the underlying @ref org.qore.AbstractDatasource "AbstractDatasource" object that must be released by calling @ref org.qore.AbstractDatasource::commit() "commit()" or @ref org.qore.AbstractDatasource::rollback() "rollback()", even if the statement does not acquire any database locks

        @see getStatementNoExec()

        @since %SqlUtil 1.5
     */
    public AbstractSQLStatement getStatement() throws Throwable {
        return new AbstractSQLStatement((QoreObject)obj.callMethodSave("getStatement"));
    }

    //! returns an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments; the statement is only prepared and not executed
    /** @par Example:
        @code{.py}
AbstractSQLStatement i = table.getStatementNoExec(sh, opts);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param opt optional SQL data operation callback options; see @ref SqlDataCallbackOptions for more info

        @return an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - the @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object created by a successful call to this method acquires a thread resource for the underlying @ref org.qore.AbstractDatasource "AbstractDatasource" object that must be released by calling @ref org.qore.AbstractDatasource::commit() "commit()" or @ref org.qore.AbstractDatasource::rollback() "rollback()", even if the statement does not acquire any database locks

        @see getStatement()

        @since %SqlUtil 1.5
     */
    public AbstractSQLStatement getStatementNoExec(Map<String, Object> sh, Map<String, Object> opt) throws Throwable {
        return new AbstractSQLStatement((QoreObject)obj.callMethodSave("getStatementNoExec", sh, opt));
    }

    //! returns an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments; the statement is only prepared and not executed
    /** @par Example:
        @code{.py}
AbstractSQLStatement i = table.getStatementNoExec(sh);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument

        @return an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate the results of a select statement matching the arguments

        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - the @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object created by a successful call to this method acquires a thread resource for the underlying @ref org.qore.AbstractDatasource "AbstractDatasource" object that must be released by calling @ref org.qore.AbstractDatasource::commit() "commit()" or @ref org.qore.AbstractDatasource::rollback() "rollback()", even if the statement does not acquire any database locks

        @see getStatement()

        @since %SqlUtil 1.5
     */
    public AbstractSQLStatement getStatementNoExec(Map<String, Object> sh) throws Throwable {
        return new AbstractSQLStatement((QoreObject)obj.callMethodSave("getStatementNoExec", sh));
    }

    //! returns an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate all the rows in the table; the statement is only prepared and not executed
    /** @par Example:
        @code{.py}
AbstractSQLStatement i = table.getStatementNoExec();
        @endcode

        @return an @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object that will iterate all the rows in the table

        @note
        - the @ref org.qore.AbstractSQLStatement "AbstractSQLStatement" object created by a successful call to this method acquires a thread resource for the underlying @ref org.qore.AbstractDatasource "AbstractDatasource" object that must be released by calling @ref org.qore.AbstractDatasource::commit() "commit()" or @ref org.qore.AbstractDatasource::rollback() "rollback()", even if the statement does not acquire any database locks

        @see getStatement()

        @since %SqlUtil 1.5
     */
    public AbstractSQLStatement getStatementNoExec() throws Throwable {
        return new AbstractSQLStatement((QoreObject)obj.callMethodSave("getStatementNoExec"));
    }

    //! returns a hash representing the row in the table that matches the argument hash; if more than one row would be returned an exception is raised
    /** @par Example:
        @code{.py}
Map<String, Object> h = table.selectRow(sh);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param opt optional SQL data operation callback options; see @ref SqlDataCallbackOptions for more info

        @return a hash representing the row in the table that matches the argument hash; if more than one row would be returned an exception is raised

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint
        @throw DBI-SELECT-ROW-ERROR more than 1 row retrieved from the server

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> selectRow(Map<String, Object> sh, Map<String, Object> opt) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("selectRow", sh, opt);
    }

    //! returns a hash representing the row in the table that matches the argument hash; if more than one row would be returned an exception is raised
    /** @par Example:
        @code{.py}
Map<String, Object> h = table.selectRow(sh);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument

        @return a hash representing the row in the table that matches the argument hash; if more than one row would be returned an exception is raised

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint
        @throw DBI-SELECT-ROW-ERROR more than 1 row retrieved from the server

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> selectRow(Map<String, Object> sh) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("selectRow", sh);
    }

    //! returns a list of hashes representing the rows in the table that match the argument hash
    /** @par Example:
        @code{.py}
Map<String, Object>[] l = table.selectRows(sh, opt);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param opt optional SQL data operation callback options; see @ref SqlDataCallbackOptions for more info

        @return a list of hashes representing the rows in the table that match the argument hash

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] selectRows(Map<String, Object> sh, Map<String, Object> opt) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("selectRows", sh, opt);
    }

    //! returns a list of hashes representing the rows in the table that match the argument hash
    /** @par Example:
        @code{.py}
Map<String, Object>[] l = table.selectRows(sh);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument

        @return a list of hashes representing the rows in the table that match the argument hash

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] selectRows(Map<String, Object> sh) throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("selectRows", sh);
    }

    //! returns a list of hashes representing the rows in the table that match the argument hash
    /** @par Example:
        @code{.py}
Map<String, Object>[] l = table.selectRows();
        @endcode

        @return a list of hashes representing the rows in the table that match the argument hash

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object>[] selectRows() throws Throwable {
        return (HashMap<String, Object>[])obj.callMethod("selectRows");
    }

    //! returns a hash of lists representing the columns and rows in the table that match the argument hahs
    /** @par Example:
        @code{.py}
Map<String, Object> h = table.select(sh);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument
        @param opt optional SQL data operation callback options; see @ref SqlDataCallbackOptions for more info

        @return a hash of lists representing the columns and rows in the table that match the argument hash

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> select(Map<String, Object> sh, Map<String, Object> opt) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("select", sh, opt);
    }

    //! returns a hash of lists representing the columns and rows in the table that match the argument hahs
    /** @par Example:
        @code{.py}
Map<String, Object> h = table.select(sh);
        @endcode

        @param sh a hash of conditions for the select statement; see @ref select_option_hash "select option hash" for information about this argument

        @return a hash of lists representing the columns and rows in the table that match the argument hash

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> select(Map<String, Object> sh) throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("select", sh);
    }

    //! returns a hash of lists representing the columns and rows in the table that match the argument hahs
    /** @par Example:
        @code{.py}
Map<String, Object> h = table.select();
        @endcode

        @return a hash of lists representing the columns and rows in the table that match the argument hash

        @throw OPTION-ERROR invalid or unsupported select option
        @throw SELECT-ERROR \c 'offset' supplied without \c 'orderby' or \c 'limit', \c 'orderby' with \c 'limit' and \c 'offset' does not match any unique constraint

        @note
        - if \c "offset" is supplied and no \c "orderby" is supplied, then if any primary key exists, the primary key columns will be used for the \c "orderby" option automatically
        - if the \c forupdate @ref select_option_hash "select option" is used, then after a successful select operation, the calling thread will own the thread transaction lock
     */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> select() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("select");
    }
}

