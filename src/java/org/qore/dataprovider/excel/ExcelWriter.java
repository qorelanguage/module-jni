/*  ExcelWriter.java Copyright 2021 - 2026 Qore Technologies, s.r.o.

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

import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Cell;
import org.apache.poi.ss.usermodel.CellStyle;
import org.apache.poi.ss.usermodel.DataFormat;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.xssf.streaming.SXSSFWorkbook;

import java.io.File;
import java.io.FileOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.Closeable;

import java.time.ZonedDateTime;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

import org.qore.jni.Hash;
import org.qore.dataprovider.format.SpreadsheetFormatCode;
import org.qore.dataprovider.format.SpreadsheetSheetName;

/**
 * Helper class for writing Excel files from Qore.
 *
 * A workbook has one or more worksheets, each with its own columns, column formats, header text, header row setting
 * and rows; the methods without a sheet index write the first worksheet.
 */
public class ExcelWriter implements Closeable {
    /** The state of a worksheet being written */
    private static class SheetState {
        final Sheet sheet;
        // the display format of each column by header name
        LinkedHashMap<String, SpreadsheetFormatCode> columnFormats = new LinkedHashMap<String, SpreadsheetFormatCode>();
        // the format code and cell style of each column by position, set when the columns are known
        SpreadsheetFormatCode[] columnCodes;
        CellStyle[] columnStyles;
        ArrayList<String> headers = new ArrayList<String>();
        int currentRow = 0;
        boolean headersWritten = false;
        // the header text of columns whose header is not their name, by column name
        HashMap<String, String> columnHeaders = new HashMap<String, String>();
        // false if no header row is written
        boolean writeHeaderRow = true;

        SheetState(Sheet sheet) {
            this.sheet = sheet;
        }
    }

    private Workbook workbook;
    private CellStyle dateStyle;
    private DataFormat dataFormat;
    // one cell style per distinct format code in the workbook, because a workbook can hold only a limited number of
    // cell styles
    private HashMap<String, CellStyle> formatStyles = new HashMap<String, CellStyle>();
    // the worksheets in workbook order
    private ArrayList<SheetState> sheets = new ArrayList<SheetState>();
    private String format;
    private boolean streaming;

    /**
     * Creates a new ExcelWriter.
     *
     * @param sheet_name The name of the first worksheet
     * @param format The output format: "xlsx" (default) or "xls"
     * @param streaming Whether to use streaming mode (SXSSF) for large files
     */
    public ExcelWriter(String sheet_name, String format, boolean streaming) {
        this.format = format != null ? format.toLowerCase() : "xlsx";
        this.streaming = streaming;

        if (this.format.equals("xls")) {
            workbook = new HSSFWorkbook();
        } else if (streaming) {
            // SXSSF for streaming large xlsx files - keeps only 100 rows of each worksheet in memory
            workbook = new SXSSFWorkbook(100);
        } else {
            workbook = new XSSFWorkbook();
        }

        try {
            addSheet(sheet_name != null && !sheet_name.isEmpty() ? sheet_name : "Sheet1");

            // Create date style
            dateStyle = workbook.createCellStyle();
            dataFormat = workbook.createDataFormat();
            dateStyle.setDataFormat(dataFormat.getFormat("yyyy-mm-dd hh:mm:ss"));
        } catch (RuntimeException e) {
            // the workbook is released, as the caller gets no writer to close: a streamed workbook has temporary files
            try {
                close();
            } catch (IOException ce) {
                e.addSuppressed(ce);
            }
            throw e;
        }
    }

    /**
     * Creates a new ExcelWriter with default settings.
     */
    public ExcelWriter() {
        this("Sheet1", "xlsx", false);
    }

    /**
     * Returns why a display format code is not accepted, or null if it is
     *
     * @param code the format code, such as {@code dd.mm.yyyy}, {@code #,##0.00} or {@code 0.0%}
     * @return the reason the format code is not accepted, or null if it is valid
     */
    public static String formatCodeIssue(String code) {
        return SpreadsheetFormatCode.issue(code);
    }

    /**
     * Returns why a worksheet name is not accepted, or null if it is
     *
     * @param name the worksheet name
     * @return the reason the name is not accepted, or null if it is valid; see {@link SpreadsheetSheetName}
     */
    public static String sheetNameIssue(String name) {
        return SpreadsheetSheetName.issue(name, true);
    }

    /**
     * Returns the key of a worksheet name that two names that differ only in case share
     *
     * @param name the worksheet name
     * @return the key of the name; two worksheets of a workbook cannot have names with the same key
     */
    public static String sheetNameKey(String name) {
        return SpreadsheetSheetName.key(name);
    }

    /**
     * Adds a worksheet after the others
     *
     * @param name the name of the worksheet; see {@link #sheetNameIssue(String)}
     * @return the 0-based index of the worksheet
     * @throws IllegalArgumentException the name is not valid, or the workbook already has a worksheet with the name
     */
    public int addSheet(String name) {
        String issue = sheetNameIssue(name);
        if (issue != null) {
            throw new IllegalArgumentException(String.format("invalid worksheet name \"%s\": %s", name, issue));
        }
        sheets.add(new SheetState(workbook.createSheet(name)));
        return sheets.size() - 1;
    }

    /**
     * Returns the number of worksheets
     */
    public int getSheetCount() {
        return sheets.size();
    }

    /**
     * Returns the state of a worksheet
     *
     * @throws IllegalArgumentException there is no worksheet with the index
     */
    private SheetState getSheet(int sheet) {
        if (sheet < 0 || sheet >= sheets.size()) {
            throw new IllegalArgumentException(String.format("there is no worksheet %d; the workbook has %d", sheet,
                sheets.size()));
        }
        return sheets.get(sheet);
    }

    /**
     * Sets the display format of the columns of the first worksheet
     *
     * @see #setColumnFormats(int, Hash)
     */
    public void setColumnFormats(Hash formats) {
        setColumnFormats(0, formats);
    }

    /**
     * Sets the display format of the columns of a worksheet
     *
     * Each value in a column is written as its type (a number stays a number, a date a date) and shown with the
     * column's format code; a value that the code does not show (a string in a number column, for example) is
     * written as it is without the format.  A column without a format code is written as before: a date with the
     * default date and time format and a number without a format.
     *
     * @param sheet the 0-based index of the worksheet
     * @param formats the format code of each column by its header name; see {@link SpreadsheetFormatCode}
     * @throws IllegalArgumentException a format code is not valid, or rows have already been written; the message
     * names the column
     */
    public void setColumnFormats(int sheet, Hash formats) {
        SheetState state = getSheet(sheet);
        if (state.currentRow > 0) {
            throw new IllegalArgumentException("column formats must be set before any row is written");
        }
        LinkedHashMap<String, SpreadsheetFormatCode> codes = new LinkedHashMap<String, SpreadsheetFormatCode>();
        for (Map.Entry<String, Object> e : formats.entrySet()) {
            Object value = e.getValue();
            if (!(value instanceof String)) {
                throw new IllegalArgumentException(String.format("the format of column \"%s\" is not a string",
                    e.getKey()));
            }
            try {
                codes.put(e.getKey(), SpreadsheetFormatCode.parse((String) value));
            } catch (IllegalArgumentException ex) {
                throw new IllegalArgumentException(String.format("column \"%s\": %s", e.getKey(), ex.getMessage()),
                    ex);
            }
        }
        state.columnFormats = codes;
    }

    /**
     * Resolves the format code and cell style of each column of a worksheet once its columns are known
     *
     * @throws IllegalArgumentException a column format names a column that is not written
     */
    private void resolveColumnStyles(SheetState state) {
        if (state.columnStyles != null) {
            return;
        }
        for (String column : state.columnFormats.keySet()) {
            if (!state.headers.contains(column)) {
                throw new IllegalArgumentException(String.format("a format is given for column \"%s\", which is not "
                    + "one of the columns written: %s", column, state.headers));
            }
        }
        SpreadsheetFormatCode[] codes = new SpreadsheetFormatCode[state.headers.size()];
        CellStyle[] styles = new CellStyle[state.headers.size()];
        for (int i = 0; i < state.headers.size(); ++i) {
            SpreadsheetFormatCode code = state.columnFormats.get(state.headers.get(i));
            if (code == null) {
                continue;
            }
            CellStyle style = formatStyles.get(code.getCode());
            if (style == null) {
                style = workbook.createCellStyle();
                style.setDataFormat(dataFormat.getFormat(code.getCode()));
                formatStyles.put(code.getCode(), style);
            }
            codes[i] = code;
            styles[i] = style;
        }
        state.columnCodes = codes;
        state.columnStyles = styles;
    }

    /**
     * Sets the headers for the first worksheet.
     *
     * @param headers Array of header names
     */
    public void setHeaders(String[] headers) {
        ArrayList<String> list = new ArrayList<String>();
        for (String h : headers) {
            list.add(h);
        }
        setHeaders(0, list);
    }

    /**
     * Sets the headers for the first worksheet.
     *
     * @param headers ArrayList of header names
     */
    public void setHeaders(ArrayList<String> headers) {
        setHeaders(0, headers);
    }

    /**
     * Sets the headers (the columns) of a worksheet
     *
     * @param sheet the 0-based index of the worksheet
     * @param headers the names of the columns, in order
     * @throws IllegalArgumentException rows have already been written
     */
    public void setHeaders(int sheet, ArrayList<String> headers) {
        SheetState state = getSheet(sheet);
        if (state.currentRow > 0) {
            throw new IllegalArgumentException("the columns must be set before any row is written");
        }
        state.headers = new ArrayList<String>(headers);
    }

    /**
     * Sets the header text of the columns of the first worksheet whose header is not their name
     *
     * @see #setColumnHeaders(int, Hash)
     */
    public void setColumnHeaders(Hash headers) {
        setColumnHeaders(0, headers);
    }

    /**
     * Sets the header text of the columns of a worksheet whose header is not their name
     *
     * @param sheet the 0-based index of the worksheet
     * @param headers the header text of each column by its name; the records are written by column name
     * @throws IllegalArgumentException a header text is not a string, or rows have already been written
     */
    public void setColumnHeaders(int sheet, Hash headers) {
        SheetState state = getSheet(sheet);
        if (state.currentRow > 0) {
            throw new IllegalArgumentException("column headers must be set before any row is written");
        }
        HashMap<String, String> names = new HashMap<String, String>();
        for (Map.Entry<String, Object> e : headers.entrySet()) {
            Object value = e.getValue();
            if (!(value instanceof String)) {
                throw new IllegalArgumentException(String.format("the header of column \"%s\" is not a string",
                    e.getKey()));
            }
            names.put(e.getKey(), (String) value);
        }
        state.columnHeaders = names;
    }

    /**
     * Sets whether a header row is written in the first worksheet
     *
     * @see #setWriteHeaders(int, boolean)
     */
    public void setWriteHeaders(boolean write) {
        setWriteHeaders(0, write);
    }

    /**
     * Sets whether a header row is written in a worksheet
     *
     * @param sheet the 0-based index of the worksheet
     * @param write false to write no header row: the first record is written in the first row
     * @throws IllegalArgumentException rows have already been written
     */
    public void setWriteHeaders(int sheet, boolean write) {
        SheetState state = getSheet(sheet);
        if (state.currentRow > 0) {
            throw new IllegalArgumentException("the header row must be set before any row is written");
        }
        state.writeHeaderRow = write;
    }

    /**
     * Writes the header row of the first worksheet, unless it is disabled with {@link #setWriteHeaders(boolean)}
     */
    public void writeHeaders() {
        writeHeaders(getSheet(0));
    }

    /**
     * Writes the header row of a worksheet whose columns are known, unless it is disabled
     */
    private void writeHeaders(SheetState state) {
        if (state.headersWritten || state.headers.isEmpty()) {
            return;
        }
        if (!state.writeHeaderRow) {
            state.headersWritten = true;
            return;
        }
        Row row = state.sheet.createRow(state.currentRow++);
        for (int i = 0; i < state.headers.size(); i++) {
            Cell cell = row.createCell(i);
            cell.setCellValue(state.columnHeaders.getOrDefault(state.headers.get(i), state.headers.get(i)));
        }
        state.headersWritten = true;
    }

    /**
     * Writes a data row to the first worksheet
     *
     * @param data The hash containing the row data
     */
    public void writeRow(Hash data) {
        writeRow(0, data);
    }

    /**
     * Writes a data row to a worksheet
     *
     * Without columns, the keys of the first row written to the worksheet are its columns; a column that the row has
     * no value for is an empty cell, and values that are not in a column are not written.
     *
     * @param sheet the 0-based index of the worksheet
     * @param data the values of the row by column name
     */
    public void writeRow(int sheet, Hash data) {
        SheetState state = getSheet(sheet);
        // With no predefined headers, the first record's keys define the columns; they are taken before any row is
        // written, so the header row is always written first and no rows ever need to be shifted (SXSSF streaming
        // sheets cannot shift rows that were already flushed)
        if (state.headers.isEmpty()) {
            for (Object key : data.keySet()) {
                state.headers.add(key.toString());
            }
        }
        resolveColumnStyles(state);
        writeHeaders(state);

        Row row = state.sheet.createRow(state.currentRow++);
        for (int i = 0; i < state.headers.size(); i++) {
            Object value = data.get(state.headers.get(i));
            Cell cell = row.createCell(i);
            setCellValue(cell, value);
            if (state.columnCodes[i] != null && state.columnCodes[i].shows(value)) {
                cell.setCellStyle(state.columnStyles[i]);
            }
        }
    }

    /**
     * Writes the header rows of worksheets without rows whose columns are known, before the workbook is written
     */
    private void finish() {
        for (SheetState state : sheets) {
            writeHeaders(state);
        }
    }

    /**
     * Sets the cell value based on the object type.
     */
    private void setCellValue(Cell cell, Object value) {
        if (value == null) {
            cell.setBlank();
        } else if (value instanceof Boolean) {
            cell.setCellValue((Boolean) value);
        } else if (value instanceof Number) {
            cell.setCellValue(((Number) value).doubleValue());
        } else if (value instanceof ZonedDateTime) {
            // the clock time of the value, as a cell has no time zone, as in the ODS writer
            cell.setCellValue(((ZonedDateTime) value).toLocalDateTime());
            cell.setCellStyle(dateStyle);
        } else if (value instanceof Date) {
            cell.setCellValue((Date) value);
            cell.setCellStyle(dateStyle);
        } else {
            cell.setCellValue(value.toString());
        }
    }

    /**
     * Writes the workbook to a file.
     *
     * @param path The file path to write to
     */
    public void writeToFile(String path) throws IOException {
        finish();
        try (FileOutputStream out = new FileOutputStream(new File(path))) {
            workbook.write(out);
        }
    }

    /**
     * Writes the workbook to an output stream.
     *
     * @param stream The output stream to write to
     */
    public void writeToStream(OutputStream stream) throws IOException {
        finish();
        workbook.write(stream);
    }

    /**
     * Writes the workbook to a Qore output stream.
     *
     * @param stream The Qore output stream to write to
     */
    public void writeToStream(qore.Qore.OutputStream stream) throws IOException {
        writeToStream(new org.qore.jni.QoreOutputStreamWrapper(stream));
    }

    /**
     * Returns the workbook contents as bytes.
     *
     * @return The workbook as a byte array
     */
    public byte[] getBytes() throws IOException {
        finish();
        try (ByteArrayOutputStream out = new ByteArrayOutputStream()) {
            workbook.write(out);
            return out.toByteArray();
        }
    }

    /**
     * Gets the current row count of the first worksheet (including header if written).
     */
    public int getRowCount() {
        return getRowCount(0);
    }

    /**
     * Gets the current row count of a worksheet (including header if written).
     *
     * @param sheet the 0-based index of the worksheet
     */
    public int getRowCount(int sheet) {
        return getSheet(sheet).currentRow;
    }

    /**
     * Closes the workbook and releases resources.
     */
    @Override
    public void close() throws IOException {
        if (workbook != null) {
            // For SXSSF, dispose of temporary files
            if (workbook instanceof SXSSFWorkbook) {
                ((SXSSFWorkbook) workbook).dispose();
            }
            workbook.close();
            workbook = null;
        }
    }
}
