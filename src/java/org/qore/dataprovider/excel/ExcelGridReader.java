/*  ExcelGridReader.java Copyright 2026 Qore Technologies, s.r.o.

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

package org.qore.dataprovider.excel;

import org.apache.poi.poifs.filesystem.FileMagic;
import org.apache.poi.ss.usermodel.Cell;
import org.apache.poi.ss.usermodel.CellType;
import org.apache.poi.ss.usermodel.DateUtil;
import org.apache.poi.ss.usermodel.Name;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.usermodel.WorkbookFactory;
import org.apache.poi.ss.util.CellRangeAddress;
import org.apache.poi.ss.util.CellReference;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.ArrayList;
import java.util.List;

import org.qore.jni.Hash;

/**
 * Reads an Excel workbook (.xlsx or .xls) as a raw grid of cells for profiling.
 *
 * The result has the shape of the Qore DataProvider::TabularWorkbookGrid hashdecl: the cell values of every
 * sheet row by row, whether each sheet is hidden, its merged ranges, and its formula text, plus the workbook's
 * named ranges.  No header or data range is assumed; the profiler finds them.
 *
 * An .xlsx workbook is read one sheet at a time as it is streamed (see XlsxSheetSource), so a workbook of any size is
 * read in bounded memory: the rows after the maximum number of rows read are counted but not kept.  An .xls workbook
 * is read into memory.  Both give the grid that POI's usermodel gives.
 */
public class ExcelGridReader {
    private static final String TEMP_PREFIX = "qore-excel-grid-";

    /**
     * Reads a workbook as a raw grid.
     *
     * @param stream the workbook content; closed when the workbook has been read
     * @param max_rows the maximum number of rows read per sheet; a sheet with more rows reports its row count
     * @return a hash with the keys format, sheets, and named_ranges
     */
    public static Hash readGrid(qore.Qore.InputStream stream, int max_rows) throws Throwable {
        return readGrid(new org.qore.jni.QoreInputStreamWrapper(stream), max_rows);
    }

    /**
     * Reads a workbook as a raw grid.
     *
     * The content is copied to a temporary file, in bounded memory, which is deleted when the workbook has been read.
     *
     * @param stream the workbook content; closed when the workbook has been read
     * @param max_rows the maximum number of rows read per sheet; a sheet with more rows reports its row count
     * @return a hash with the keys format, sheets, and named_ranges
     */
    public static Hash readGrid(InputStream stream, int max_rows) throws Throwable {
        Path tmp;
        try (InputStream in = stream) {
            checkMaxRows(max_rows);
            tmp = Files.createTempFile(TEMP_PREFIX, ".tmp");
            try (OutputStream out = Files.newOutputStream(tmp)) {
                in.transferTo(out);
            } catch (Throwable t) {
                deleteTempFile(tmp, t);
                throw t;
            }
        }
        try {
            return readGrid(tmp.toFile(), max_rows);
        } finally {
            Files.deleteIfExists(tmp);
        }
    }

    private static Hash readGrid(File file, int max_rows) throws Throwable {
        if (FileMagic.valueOf(file) == FileMagic.OOXML) {
            return readXlsxGrid(file, max_rows);
        }
        return readWorkbookGrid(file, max_rows);
    }

    private static void checkMaxRows(int max_rows) {
        if (max_rows <= 0) {
            throw new IllegalArgumentException("max_rows must be positive; got " + max_rows);
        }
    }

    private static void deleteTempFile(Path tmp, Throwable error) {
        try {
            Files.deleteIfExists(tmp);
        } catch (Throwable t) {
            error.addSuppressed(t);
        }
    }

    /**
     * Reads an .xlsx workbook as it is streamed, one sheet at a time
     */
    private static Hash readXlsxGrid(File file, int max_rows) throws Throwable {
        ZoneId zone = ZoneId.systemDefault();
        ArrayList<Hash> sheets = new ArrayList<Hash>();
        ArrayList<Hash> infos = XlsxSheetSource.getWorksheetInfo(file);
        for (int i = 0; i < infos.size(); ++i) {
            Hash info = infos.get(i);
            try (XlsxSheetSource src = XlsxSheetSource.openAt(file, i)) {
                sheets.add(readSheet(src, (String)info.get("name"), (Boolean)info.get("hidden"), max_rows, zone));
            }
        }

        // the names that XSSFWorkbook.getAllNames() gives, other than function names and names without a formula
        Hash named_ranges = new Hash();
        for (String[] name : XlsxSheetSource.getDefinedNames(file)) {
            String refers_to = name[3];
            if (name[0] != null && refers_to != null && !refers_to.isEmpty() && !isTrue(name[2])) {
                named_ranges.put(name[0], refers_to);
            }
        }

        Hash rv = new Hash();
        rv.put("format", "xlsx");
        rv.put("sheets", sheets);
        rv.put("named_ranges", named_ranges);
        return rv;
    }

    /**
     * Returns true for an xsd:boolean attribute value that is true
     */
    private static boolean isTrue(String value) {
        if (value == null) {
            return false;
        }
        String v = value.trim();
        return "1".equals(v) || "true".equals(v);
    }

    /**
     * Reads a sheet of an .xlsx workbook in one pass over its rows: the rows up to the maximum are kept, and the
     * others are counted
     */
    private static Hash readSheet(XlsxSheetSource src, String name, boolean hidden, int max_rows, ZoneId zone)
            throws IOException {
        src.enableFormulas();
        src.enableMergedRegions();

        ArrayList<ArrayList<Object>> rows = new ArrayList<ArrayList<Object>>();
        Hash formulas = new Hash();
        // the number of rows: the 0-based number of the last physical row plus one
        int row_count = 0;
        try (SheetSource.RowReader reader = src.openRows()) {
            for (SheetSource.Row row = reader.next(); row != null; row = reader.next()) {
                int r = row.getRowNum();
                row_count = r + 1;
                if (r >= max_rows) {
                    continue;
                }
                // a row without a row element is empty
                while (rows.size() < r) {
                    rows.add(new ArrayList<Object>());
                }
                ArrayList<Object> values = new ArrayList<Object>();
                for (int c = 0, last = row.getLastCellNum(); c < last; ++c) {
                    SheetSource.Cell cell = row.getCell(c);
                    if (cell == null) {
                        values.add(null);
                        continue;
                    }
                    if (cell.getCellType() == CellType.FORMULA) {
                        formulas.put(new CellReference(r, c).formatAsString(), cell.getCellFormula());
                    }
                    values.add(cellToValue(cell, zone));
                }
                rows.add(values);
            }
        }
        int read_rows = Math.min(row_count, max_rows);
        while (rows.size() < read_rows) {
            rows.add(new ArrayList<Object>());
        }

        Hash rv = new Hash();
        rv.put("name", name);
        rv.put("hidden", hidden);
        rv.put("rows", rows);
        if (row_count > read_rows) {
            rv.put("row_count", row_count);
        }
        rv.put("merged", getMergedHashes(src.getMergedRegions()));
        rv.put("formulas", formulas);
        return rv;
    }

    private static ArrayList<Hash> getMergedHashes(List<CellRangeAddress> regions) {
        ArrayList<Hash> merged = new ArrayList<Hash>();
        for (CellRangeAddress range : regions) {
            Hash h = new Hash();
            h.put("first_row", range.getFirstRow());
            h.put("last_row", range.getLastRow());
            h.put("first_col", range.getFirstColumn());
            h.put("last_col", range.getLastColumn());
            merged.add(h);
        }
        return merged;
    }

    private static Object cellToValue(SheetSource.Cell cell, ZoneId zone) {
        CellType type = cell.getCellType();
        if (type == CellType.FORMULA) {
            type = cell.getCachedFormulaResultType();
        }
        switch (type) {
            case BOOLEAN:
                return cell.getBooleanCellValue();
            case NUMERIC:
                if (cell.isCellDateFormatted()) {
                    return ZonedDateTime.of(cell.getLocalDateTimeCellValue(), zone);
                }
                return cell.getNumericCellValue();
            case STRING:
                return cell.getStringCellValue();
            default:
                return null;
        }
    }

    /**
     * Reads a workbook file of any format into memory as a raw grid with POI's usermodel; .xls workbooks are read
     * this way, and the grid of an .xlsx workbook read as it is streamed is the same
     *
     * @param file the workbook file
     * @param max_rows the maximum number of rows read per sheet; a sheet with more rows reports its row count
     * @return a hash with the keys format, sheets, and named_ranges
     */
    static Hash readWorkbookGrid(File file, int max_rows) throws Throwable {
        try (Workbook wb = WorkbookFactory.create(file, null, true)) {
            ZoneId zone = ZoneId.systemDefault();
            ArrayList<Hash> sheets = new ArrayList<Hash>();
            for (int i = 0; i < wb.getNumberOfSheets(); ++i) {
                sheets.add(readSheet(wb, i, max_rows, zone));
            }

            Hash named_ranges = new Hash();
            for (Name name : wb.getAllNames()) {
                String refers_to = name.getRefersToFormula();
                if (name.getNameName() != null && refers_to != null && !name.isFunctionName()) {
                    named_ranges.put(name.getNameName(), refers_to);
                }
            }

            Hash rv = new Hash();
            rv.put("format", wb instanceof XSSFWorkbook ? "xlsx" : "xls");
            rv.put("sheets", sheets);
            rv.put("named_ranges", named_ranges);
            return rv;
        }
    }

    private static Hash readSheet(Workbook wb, int index, int max_rows, ZoneId zone) {
        Sheet sheet = wb.getSheetAt(index);
        int row_count = sheet.getPhysicalNumberOfRows() == 0 ? 0 : sheet.getLastRowNum() + 1;
        int read_rows = Math.min(row_count, max_rows);

        ArrayList<ArrayList<Object>> rows = new ArrayList<ArrayList<Object>>(read_rows);
        Hash formulas = new Hash();
        for (int r = 0; r < read_rows; ++r) {
            ArrayList<Object> values = new ArrayList<Object>();
            Row row = sheet.getRow(r);
            if (row != null) {
                int last_cell = row.getLastCellNum();
                for (int c = 0; c < last_cell; ++c) {
                    Cell cell = row.getCell(c);
                    if (cell == null) {
                        values.add(null);
                        continue;
                    }
                    if (cell.getCellType() == CellType.FORMULA) {
                        formulas.put(new CellReference(r, c).formatAsString(), cell.getCellFormula());
                    }
                    values.add(cellToValue(cell, zone));
                }
            }
            rows.add(values);
        }

        Hash rv = new Hash();
        rv.put("name", wb.getSheetName(index));
        rv.put("hidden", wb.isSheetHidden(index) || wb.isSheetVeryHidden(index));
        rv.put("rows", rows);
        if (row_count > read_rows) {
            rv.put("row_count", row_count);
        }
        rv.put("merged", getMergedHashes(sheet.getMergedRegions()));
        rv.put("formulas", formulas);
        return rv;
    }

    private static Object cellToValue(Cell cell, ZoneId zone) {
        CellType type = cell.getCellType();
        if (type == CellType.FORMULA) {
            type = cell.getCachedFormulaResultType();
        }
        switch (type) {
            case BOOLEAN:
                return cell.getBooleanCellValue();
            case NUMERIC:
                if (DateUtil.isCellDateFormatted(cell)) {
                    return ZonedDateTime.of(cell.getLocalDateTimeCellValue(), zone);
                }
                return cell.getNumericCellValue();
            case STRING:
                return cell.getStringCellValue();
            default:
                return null;
        }
    }
}
