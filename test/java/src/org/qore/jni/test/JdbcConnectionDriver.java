// Copyright (C) 2026 Qore Technologies, s.r.o.; SPDX-License-Identifier: MIT
package org.qore.jni.test;

import java.io.IOException;
import java.net.Socket;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Proxy;
import java.sql.*;
import java.util.Properties;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.logging.Logger;

/** A real socket handshake with deterministic success/error controls and connection ownership counters. */
public final class JdbcConnectionDriver implements Driver {
    private static boolean installed;
    private static final AtomicInteger active = new AtomicInteger();
    private static final AtomicInteger closed = new AtomicInteger();
    public static synchronized void install() throws SQLException {
        if (!installed) {
            DriverManager.registerDriver(new JdbcConnectionDriver());
            installed = true;
        }
    }
    public static int active() { return active.get(); }
    public static int closed() { return closed.get(); }
    @Override public boolean acceptsURL(String url) { return url.startsWith("jdbc:qore-connect:"); }
    @Override public Connection connect(String url, Properties properties) throws SQLException {
        if (!acceptsURL(url)) {
            return null;
        }
        String[] fields = url.split(":");
        int port = Integer.parseInt(fields[2]);
        active.incrementAndGet();
        try (Socket socket = new Socket("127.0.0.1", port)) {
            if (!Thread.currentThread().isVirtual()) {
                throw new SQLException("connection must run on a virtual thread");
            }
            socket.setSoTimeout(Integer.parseInt(fields[3]));
            int response = socket.getInputStream().read();
            if (response < 0) {
                throw new SQLException("fixture connection closed");
            }
            return openConnection(response);
        } catch (IOException error) {
            if (fields.length > 4 && fields[4].equals("return-on-interrupt")) {
                return openConnection(1);
            }
            throw new SQLException("fixture connection I/O failed", error);
        } finally {
            active.decrementAndGet();
        }
    }
    /** Exercise exception conversion on the calling Qore thread with cancellation pending. */
    public static void throwAfterRead(int port) throws IOException, SQLException {
        try (Socket socket = new Socket("127.0.0.1", port)) {
            socket.setSoTimeout(5000);
            socket.getInputStream().read();
            throw new SQLException("fixture exception after cancellation");
        }
    }
    private static Connection openConnection(int response) throws SQLException {
            Connection delegate = JdbcTransactionDriver.open();
            return (Connection) Proxy.newProxyInstance(JdbcConnectionDriver.class.getClassLoader(),
                new Class<?>[]{Connection.class}, (object, method, args) -> {
                    if (method.getName().equals("getMetaData") && response == 2) {
                        throw new SQLException("fixture metadata failure");
                    }
                    if (method.getName().equals("setAutoCommit") && response == 3) {
                        throw new SQLException("fixture initialization failure");
                    }
                    if (method.getName().equals("close")) {
                        closed.incrementAndGet();
                    }
                    try {
                        return method.invoke(delegate, args);
                    } catch (InvocationTargetException error) {
                        throw error.getCause();
                    }
                });
    }
    @Override public DriverPropertyInfo[] getPropertyInfo(String url, Properties properties) {
        return new DriverPropertyInfo[0];
    }
    @Override public int getMajorVersion() { return 1; }
    @Override public int getMinorVersion() { return 0; }
    @Override public boolean jdbcCompliant() { return false; }
    @Override public Logger getParentLogger() throws SQLFeatureNotSupportedException {
        throw new SQLFeatureNotSupportedException();
    }
}
