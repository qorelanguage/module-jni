package org.qore.lang.test;

import org.qore.lang.*;
import org.qore.lang.sqlutil.*;
import org.qore.lang.bulksqlutil.*;

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
        Object[] rv = new Object[2];
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
        BulkInsertOperation insert = new BulkInsertOperation(table);
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
        return rv;
    }
}
