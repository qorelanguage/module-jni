// Copyright (C) 2026 Qore Technologies, s.r.o.
// SPDX-License-Identifier: MIT
package org.qore.jni.test;

import java.lang.reflect.Proxy;
import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.DriverPropertyInfo;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.util.ArrayList;
import java.util.Properties;
import java.util.logging.Logger;

/** Instrumented JDBC driver for transaction-boundary and failure-path tests. */
public final class JdbcTransactionDriver implements Driver {
    // All fixture state and proxy callbacks are serialized on this class's monitor.
    private static final ArrayList<String> events = new ArrayList<>();
    private static String failure = "";
    // execution at which the connection is lost, and whether the server applied the statement first
    private static String loss = "";
    private static boolean lossApplied;
    private static int durable;
    private static boolean installed;

    public static synchronized void reset() throws SQLException {
        if (!installed) {
            DriverManager.registerDriver(new JdbcTransactionDriver());
            installed = true;
        }
        events.clear();
        failure = "";
        loss = "";
        lossApplied = false;
        durable = 0;
    }

    public static synchronized void failNext(String operation) {
        failure = operation;
    }

    /** The connection is lost at the given "execute:<sql>:<auto-commit>" event; with {@code applied} the server
        executed the statement before the connection dropped (committing it in auto-commit mode). */
    public static synchronized void loseConnectionAt(String operation, boolean applied) {
        loss = operation;
        lossApplied = applied;
    }

    public static synchronized int count(String operation) {
        int rv = 0;
        for (String e : events) {
            if (e.equals(operation)) {
                ++rv;
            }
        }
        return rv;
    }

    public static synchronized String[] events() {
        return events.toArray(new String[0]);
    }

    public static synchronized int durable() {
        return durable;
    }

    private static void event(String operation) throws SQLException {
        events.add(operation);
        if (failure.equals(operation)) {
            failure = "";
            throw new SQLException("injected " + operation);
        }
    }

    private static final class Session {
        boolean autoCommit = true;
        boolean closed;
        int pending;

        PreparedStatement prepare(String sql) {
            // Accessed only by the synchronized proxy callbacks below.
            int[] batchSize = {0};
            // the update count is only available after a successful execution
            int[] updateCount = {-1};
            return (PreparedStatement) Proxy.newProxyInstance(JdbcTransactionDriver.class.getClassLoader(),
                new Class<?>[] {PreparedStatement.class}, (proxy, method, args) -> {
                    synchronized (JdbcTransactionDriver.class) {
                        switch (method.getName()) {
                            case "execute": {
                                String operation = "execute:" + sql + ":" + autoCommit;
                                boolean lose = loss.equals(operation);
                                if (lose) {
                                    loss = "";
                                }
                                event(operation);
                                if (sql.equals("write") && (!lose || lossApplied)) {
                                    if (autoCommit) {
                                        ++durable;
                                    } else {
                                        ++pending;
                                    }
                                }
                                if (lose) {
                                    // the server rolls back the open transaction of a lost session
                                    pending = 0;
                                    closed = true;
                                    throw new SQLException("injected connection loss", "08006");
                                }
                                updateCount[0] = 1;
                                return false;
                            }
                            case "getUpdateCount": return updateCount[0];
                            case "setByte":
                            case "setShort":
                            case "setInt":
                            case "setLong":
                            case "setNull": return null;
                            case "clearBatch": batchSize[0] = 0; return null;
                            case "addBatch": ++batchSize[0]; return null;
                            case "executeBatch":
                                int[] counts = new int[batchSize[0]];
                                batchSize[0] = 0;
                                for (int i = 0; i < counts.length; ++i) {
                                    counts[i] = sql.startsWith("unknown") ? java.sql.Statement.SUCCESS_NO_INFO
                                        : sql.startsWith("overflow") ? Integer.MAX_VALUE
                                        : sql.startsWith("invalid") ? java.sql.Statement.EXECUTE_FAILED : i + 1;
                                }
                                return counts;
                            case "close": event("statement.close"); return null;
                            default: throw new SQLFeatureNotSupportedException(method.getName());
                        }
                    }
                });
        }

        Connection connect() {
            return (Connection) Proxy.newProxyInstance(JdbcTransactionDriver.class.getClassLoader(),
                new Class<?>[] {Connection.class}, (proxy, method, args) -> {
                    synchronized (JdbcTransactionDriver.class) {
                        switch (method.getName()) {
                            case "setAutoCommit":
                                boolean value = (Boolean) args[0];
                                event("auto:" + value);
                                if (value && !autoCommit) {
                                    durable += pending;
                                    pending = 0;
                                }
                                autoCommit = value;
                                return null;
                            case "getAutoCommit": return autoCommit;
                            case "prepareStatement": return prepare((String) args[0]);
                            case "commit":
                                event("commit");
                                if (autoCommit) {
                                    throw new SQLException("commit in auto-commit mode");
                                }
                                durable += pending;
                                pending = 0;
                                return null;
                            case "rollback":
                                event("rollback");
                                if (autoCommit) {
                                    throw new SQLException("rollback in auto-commit mode");
                                }
                                pending = 0;
                                return null;
                            case "close":
                                pending = 0;
                                closed = true;
                                event("close");
                                return null;
                            case "isClosed": return closed;
                            case "isValid": return !closed;
                            case "getMetaData": return null;
                            default: throw new SQLFeatureNotSupportedException(method.getName());
                        }
                    }
                });
        }
    }

    @Override
    public synchronized Connection connect(String url, Properties info) throws SQLException {
        synchronized (JdbcTransactionDriver.class) {
            if (!acceptsURL(url)) {
                return null;
            }
            event("connect");
            return new Session().connect();
        }
    }

    @Override public boolean acceptsURL(String url) { return url.startsWith("jdbc:qore-transaction-test:"); }
    @Override public DriverPropertyInfo[] getPropertyInfo(String url, Properties info) {
        return new DriverPropertyInfo[0];
    }
    @Override public int getMajorVersion() { return 1; }
    @Override public int getMinorVersion() { return 0; }
    @Override public boolean jdbcCompliant() { return false; }
    @Override public Logger getParentLogger() throws SQLFeatureNotSupportedException {
        throw new SQLFeatureNotSupportedException();
    }
}
