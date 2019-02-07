package org.qore.lang.test;

import org.qore.lang.*;
import org.qore.lang.sqlutil.*;
import org.qore.lang.bulksqlutil.*;
import org.qore.lang.qunit.*;

import java.util.Map;
import java.util.HashMap;
import java.time.ZonedDateTime;
import java.time.ZoneId;
import java.util.ArrayList;

public class QoreJavaLangApiTest {
    static Table createTable(AbstractDatasource ds) throws Throwable {
        Table table = new Table(ds, "test_table_1");
        HashMap<String, Object> column = new HashMap<String, Object>() {
            {
                put("qore_type", "integer");
            }
        };
        table.addColumn("id", column, false);
        column = new HashMap<String, Object>() {
            {
                put("qore_type", "string");
                put("size", 50);
            }
        };
        table.addColumn("string", column, false);
        column = new HashMap<String, Object>() {
            {
                put("qore_type", "date");
            }
        };
        table.addColumn("modified", column, false);

        // add PK
        table.addPrimaryKey("pk_test_table_1", new String[]{"id"});

        try {
            table.create();
            table.commit();
        } catch (Throwable e) {
            table.rollback();
            throw e;
        }
        return table;
    }

    static boolean dropTable(Table table) throws Throwable {
        table.drop();
        table.commit();
        return true;
    }

    static boolean testTable(Table table) throws Throwable {
        AbstractDatasource ds = table.getDatasource();
        try {
            // dates retrieved from the DB will have their region info stripped
            ZoneId zone = ZoneId.of(ZonedDateTime.now().getOffset().toString());
            final ZonedDateTime now = ZonedDateTime.now(zone);
            AbstractSQLStatement stmt = ds.getSQLStatement();
            stmt.prepare("insert into test_table_1 (id, string, modified) values(%v, %v, %v)");
            stmt.bind(1, "hello", now);
            stmt.exec();
            stmt.commit();

            if (table.rowCount() != 1) {
                throw new Throwable("rowcount");
            }

            HashMap<String, Object> wh = new HashMap<String, Object>() {
                {
                    put("id", 1);
                }
            };
            HashMap<String, Object> sh = new HashMap<String, Object>() {
                {
                    put("where", wh);
                }
            };
            HashMap row = table.selectRow(sh);
            if ((int)row.get("id") != 1) {
                throw new Throwable("id");
            }
            if (!row.get("string").equals("hello")) {
                System.out.println("STR: " + row.get("string").toString() + " != hello");
                throw new Throwable("string");
            }
            if (!row.get("modified").equals(now)) {
                throw new Throwable("modified");
            }

            return true;
        } catch (Throwable e) {
            table.rollback();
            throw e;
        } finally {
            table.commit();
        }
    }

    static Object[] testBulkInsert(Table table) throws Throwable {
        Object[] rv = new Object[3];
        // dates retrieved from the DB will have their region info stripped
        ZoneId zone = ZoneId.of(ZonedDateTime.now().getOffset().toString());
        final ZonedDateTime now = ZonedDateTime.now(zone);
        HashMap<String, Object> row = new HashMap<String, Object>() {
            {
                put("id", 2);
                put("string", "str");
                put("modified", now);
            }
        };

        ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        BulkRowCallback rowCallback = (Map<String, Object> new_row) -> list.add(new_row);

        ArrayList<String> infoList = new ArrayList<String>();
        LogCallback logCallback = (String fmt, Object... args) -> infoList.add(String.format(fmt, args));
        HashMap<String, Object> opts = new HashMap<String, Object>() {
            {
                put("info_log", logCallback);
            }
        };
        BulkInsertOperation insert = new BulkInsertOperation(table, opts);
        insert.setRowCode(rowCallback);
        try {
            insert.queueData(row);
            insert.flush();
            insert.commit();
        } catch (Throwable e) {
            table.rollback();
            throw e;
        }

        HashMap<String, Object> wh = new HashMap<String, Object>() {
            {
                put("id", 2);
            }
        };
        HashMap<String, Object> sh = new HashMap<String, Object>() {
            {
                put("where", wh);
            }
        };
        rv[0] = table.selectRow(sh);
        rv[1] = list;
        rv[2] = infoList;
        return rv;
    }

    static Map<String, Object> testBulkUpsert(Table table) throws Throwable {
        // dates retrieved from the DB will have their region info stripped
        ZoneId zone = ZoneId.of(ZonedDateTime.now().getOffset().toString());
        final ZonedDateTime now = ZonedDateTime.now(zone);
        HashMap<String, Object> row = new HashMap<String, Object>() {
            {
                put("id", 2);
                put("string", "new-str");
                put("modified", now);
            }
        };

        ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
        BulkRowCallback rowCallback = (Map<String, Object> new_row) -> list.add(new_row);
        BulkUpsertOperation insert = new BulkUpsertOperation(table);
        try {
            insert.queueData(row);
            insert.flush();
            insert.commit();
        } catch (Throwable e) {
            table.rollback();
            throw e;
        }

        HashMap<String, Object> wh = new HashMap<String, Object>() {
            {
                put("id", 2);
            }
        };
        HashMap<String, Object> sh = new HashMap<String, Object>() {
            {
                put("where", wh);
            }
        };
        return table.selectRow(sh);
    }

    static void doTests() throws Throwable {
        Test test = new Test("JniTest", "1.0");

        TestCode code = () -> {
            test.assertTrue(true);
            test.assertFalse(false);
            test.assertEq(1, 1);
            test.assertEqSoft(1, "1");
            test.assertNeq(1, 2);
            test.assertNeqSoft(1, "2");
            test.assertGt(1, 2);
            test.assertGtSoft(1, "2");
            test.assertGe(1, 2);
            test.assertGeSoft(1, "2");
            test.assertLt(2, 1);
            test.assertLtSoft(2, "1");
            test.assertLe(2, 1);
            test.assertLeSoft(2, "1");

            test.testAssertion("my-test", () -> { return true; });
            test.testAssertion("my-test", () -> { return true; }, new TestResultSuccess());
            test.testAssertion("my-test", () -> { return false; }, new TestResultFailure());
            test.testAssertion("my-test", () -> { return 1; }, new TestResultValue(1));
            {
                Object[] args = new Object[1];
                args[0] = 1;
                test.testAssertion("my-test", (Object... lambda_args) -> { return lambda_args[0]; }, args, new TestResultValue(1));
            }

            test.assertThrows("JNI-ERROR", "Error", () -> { throw new Throwable("Error"); });
            test.testAssertion("my-test", () -> { throw new Throwable("Error"); }, new TestResultExceptionType("JNI-ERROR"));
            test.testAssertion("my-test", () -> { throw new Throwable("Error"); }, new TestResultExceptionDetail("JNI-ERROR", "java.lang.Throwable: Error"));
            test.testAssertion("my-test", () -> { throw new Throwable("Error"); }, new TestResultExceptionRegexp("JNI-ERROR", "Error"));
            test.testAssertion("my-test", () -> { throw new Throwable("Error"); }, new TestResultExceptionSubstring("JNI-ERROR", "Error"));
        };
        test.addTestCase("my test", code);
        test.main();
    }
}
