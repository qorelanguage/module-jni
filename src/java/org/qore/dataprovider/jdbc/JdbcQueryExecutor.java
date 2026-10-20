/*  JdbcQueryExecutor.java Copyright 2026 Qore Technologies, s.r.o.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

package org.qore.dataprovider.jdbc;

import java.io.Closeable;
import java.io.IOException;
import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Types;
import java.time.LocalDate;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.ArrayList;

import org.qore.jni.Hash;

/**
 * Wraps a {@link java.sql.Connection} for executing parameterized queries and
 * returning results as {@link Hash} objects suitable for Qore consumption.
 * <p>
 * Implements {@link Closeable} but does <b>not</b> close the underlying
 * connection — that is owned by the caller.
 */
public class JdbcQueryExecutor implements Closeable {
    private final Connection connection;

    /**
     * Creates a new query executor for the given connection.
     *
     * @param connection the JDBC connection to use (caller retains ownership)
     */
    public JdbcQueryExecutor(Connection connection) {
        this.connection = connection;
    }

    /**
     * Executes a SELECT query and returns all rows as a list of Hash.
     * <p>
     * The SQL must return one result set; the results after it are read, so that the errors of later statements
     * are raised, and another result set raises an exception.
     *
     * @param sql the SQL SELECT statement
     * @return list of Hash entries, one per row
     * @throws SQLException if a database access error occurs, if a statement fails, or if the SQL returns more
     *         than one result set
     */
    public ArrayList<Hash> executeQuery(String sql) throws SQLException {
        try (Statement stmt = connection.createStatement()) {
            ArrayList<Hash> list;
            try (ResultSet rs = stmt.executeQuery(sql)) {
                list = readRows(rs);
            }
            readRemainingResults(stmt, true);
            return list;
        }
    }

    /**
     * Executes an INSERT, UPDATE, or DELETE statement.
     * <p>
     * Every result is read, so that the errors of later statements are raised; the rows of result sets returned
     * after the first result are read and discarded.
     *
     * @param sql the SQL DML statement
     * @return the number of affected rows: the total of the update counts of all statements
     * @throws SQLException if a database access error occurs or if a statement fails
     */
    public int executeUpdate(String sql) throws SQLException {
        try (Statement stmt = connection.createStatement()) {
            int count = stmt.executeUpdate(sql);
            return totalUpdateCount(count, readRemainingResults(stmt, false));
        }
    }

    /**
     * Executes a parameterized SELECT query and returns all rows as a list of Hash.
     * <p>
     * The SQL must return one result set; the results after it are read, so that the errors of later statements
     * are raised, and another result set raises an exception.
     *
     * @param sql    the SQL SELECT statement with {@code ?} placeholders
     * @param params the parameter values to bind
     * @return list of Hash entries, one per row
     * @throws SQLException if a database access error occurs, if a statement fails, or if the SQL returns more
     *         than one result set
     */
    public ArrayList<Hash> executeQueryWithParams(String sql, Object[] params) throws SQLException {
        try (PreparedStatement ps = connection.prepareStatement(sql)) {
            setParams(ps, params);
            ArrayList<Hash> list;
            try (ResultSet rs = ps.executeQuery()) {
                list = readRows(rs);
            }
            readRemainingResults(ps, true);
            return list;
        }
    }

    /**
     * Executes a parameterized INSERT, UPDATE, or DELETE statement.
     * <p>
     * Every result is read, so that the errors of later statements are raised; the rows of result sets returned
     * after the first result are read and discarded.
     *
     * @param sql    the SQL DML statement with {@code ?} placeholders
     * @param params the parameter values to bind
     * @return the number of affected rows: the total of the update counts of all statements
     * @throws SQLException if a database access error occurs or if a statement fails
     */
    public int executeUpdateWithParams(String sql, Object[] params) throws SQLException {
        try (PreparedStatement ps = connection.prepareStatement(sql)) {
            setParams(ps, params);
            int count = ps.executeUpdate();
            return totalUpdateCount(count, readRemainingResults(ps, false));
        }
    }

    /**
     * Reads all rows of a result set.
     *
     * @param rs the result set
     * @return list of Hash entries, one per row
     * @throws SQLException if a database access error occurs
     */
    private ArrayList<Hash> readRows(ResultSet rs) throws SQLException {
        ResultSetMetaData meta = rs.getMetaData();
        ArrayList<Hash> list = new ArrayList<>();
        while (rs.next()) {
            list.add(resultSetRowToHash(rs, meta));
        }
        return list;
    }

    /**
     * Reads the results after the first result of an executed statement.
     * <p>
     * JDBC drivers may report the error of a later statement only when its result is reached (ex: the SQL Server
     * driver), so every result is read: errors are raised, and the rows of result sets are read, as a failing
     * statement can report its error while its rows are read.
     *
     * @param stmt  the executed statement
     * @param query {@code true} if the SQL is a query, for which another result set raises an exception
     * @return the total of the update counts of the results read
     * @throws SQLException if a database access error occurs, if a statement fails, or if a query returns more than
     *         one result set
     */
    private static long readRemainingResults(Statement stmt, boolean query) throws SQLException {
        long total = 0;
        while (true) {
            if (stmt.getMoreResults()) {
                if (query) {
                    throw new SQLException("the SQL of the query returned more than one result set; a query "
                        + "returns the rows of one result set");
                }
                try (ResultSet rs = stmt.getResultSet()) {
                    while (rs.next()) {
                        // the rows are not returned; the statement's error, if any, is raised while they are read
                    }
                }
                continue;
            }
            int count = stmt.getUpdateCount();
            if (count == -1) {
                return total;
            }
            total += count;
        }
    }

    /**
     * Returns the total of the first update count and the update counts read after it.
     *
     * @param first the update count of the first statement
     * @param rest  the total of the update counts of the later statements
     * @return the total
     * @throws SQLException if the total exceeds the supported integer range
     */
    private static int totalUpdateCount(int first, long rest) throws SQLException {
        long total = first + rest;
        if (total > Integer.MAX_VALUE) {
            throw new SQLException("the total update count exceeds the supported integer range");
        }
        return (int) total;
    }

    /**
     * Sets parameters on a {@link PreparedStatement} based on the runtime type
     * of each parameter value.
     *
     * @param ps     the prepared statement
     * @param params the parameter values to bind
     * @throws SQLException if a database access error occurs
     */
    private void setParams(PreparedStatement ps, Object[] params) throws SQLException {
        if (params == null) {
            return;
        }
        for (int i = 0; i < params.length; i++) {
            int idx = i + 1;
            Object param = params[i];
            if (param == null) {
                ps.setNull(idx, Types.NULL);
            } else if (param instanceof String) {
                ps.setString(idx, (String) param);
            } else if (param instanceof Integer) {
                ps.setInt(idx, (Integer) param);
            } else if (param instanceof Long) {
                ps.setLong(idx, (Long) param);
            } else if (param instanceof Float) {
                ps.setFloat(idx, (Float) param);
            } else if (param instanceof Double) {
                ps.setDouble(idx, (Double) param);
            } else if (param instanceof BigDecimal) {
                ps.setBigDecimal(idx, (BigDecimal) param);
            } else if (param instanceof Boolean) {
                ps.setBoolean(idx, (Boolean) param);
            } else if (param instanceof byte[]) {
                ps.setBytes(idx, (byte[]) param);
            } else if (param instanceof java.sql.Date) {
                ps.setDate(idx, (java.sql.Date) param);
            } else if (param instanceof java.sql.Timestamp) {
                ps.setTimestamp(idx, (java.sql.Timestamp) param);
            } else {
                ps.setObject(idx, param);
            }
        }
    }

    /**
     * Converts the current row of a {@link ResultSet} to a {@link Hash},
     * mapping SQL types to appropriate Java types.
     *
     * @param rs   the result set positioned at the current row
     * @param meta the result-set metadata
     * @return a Hash with column names as keys
     * @throws SQLException if a database access error occurs
     */
    private Hash resultSetRowToHash(ResultSet rs, ResultSetMetaData meta) throws SQLException {
        Hash h = new Hash();
        int columnCount = meta.getColumnCount();
        for (int i = 1; i <= columnCount; i++) {
            String columnName = meta.getColumnLabel(i);
            int sqlType = meta.getColumnType(i);
            Object value;
            switch (sqlType) {
                case Types.VARCHAR:
                case Types.CHAR:
                case Types.LONGVARCHAR:
                case Types.NVARCHAR:
                case Types.NCHAR:
                case Types.CLOB:
                case Types.NCLOB:
                    value = rs.getString(i);
                    break;
                case Types.INTEGER:
                case Types.SMALLINT:
                case Types.TINYINT:
                    value = rs.getObject(i) != null ? rs.getInt(i) : null;
                    break;
                case Types.BIGINT:
                    value = rs.getObject(i) != null ? rs.getLong(i) : null;
                    break;
                case Types.FLOAT:
                case Types.REAL:
                    value = rs.getObject(i) != null ? rs.getFloat(i) : null;
                    break;
                case Types.DOUBLE:
                    value = rs.getObject(i) != null ? rs.getDouble(i) : null;
                    break;
                case Types.DECIMAL:
                case Types.NUMERIC:
                    value = rs.getBigDecimal(i);
                    break;
                case Types.BOOLEAN:
                case Types.BIT:
                    value = rs.getObject(i) != null ? rs.getBoolean(i) : null;
                    break;
                case Types.DATE:
                    java.sql.Date sqlDate = rs.getDate(i);
                    value = sqlDate != null ? sqlDate.toLocalDate() : null;
                    break;
                case Types.TIMESTAMP:
                case Types.TIMESTAMP_WITH_TIMEZONE:
                    java.sql.Timestamp ts = rs.getTimestamp(i);
                    value = ts != null
                        ? ZonedDateTime.ofInstant(ts.toInstant(), ZoneId.systemDefault())
                        : null;
                    break;
                case Types.BLOB:
                case Types.BINARY:
                case Types.VARBINARY:
                case Types.LONGVARBINARY:
                    value = rs.getBytes(i);
                    break;
                case Types.NULL:
                    value = null;
                    break;
                default:
                    value = rs.getString(i);
                    break;
            }
            h.put(columnName, value);
        }
        return h;
    }

    /**
     * No-op close — the connection is owned by the caller.
     */
    @Override
    public void close() throws IOException {
        // Connection is owned by the caller; nothing to release here.
    }
}
