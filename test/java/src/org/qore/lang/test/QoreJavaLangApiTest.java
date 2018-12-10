package org.qore.lang.test;

import org.qore.lang.*;
import org.qore.lang.sqlutil.*;

import java.util.HashMap;
import java.time.ZonedDateTime;
import java.time.ZoneId;

public class QoreJavaLangApiTest {
    static boolean testTable(AbstractDatasource ds) throws Throwable {
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
            table.drop();
            table.commit();
        }
    }
}
