/*  OdsWriter.java Copyright 2026 Qore Technologies, s.r.o.

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

package org.qore.dataprovider.ods;

import org.odftoolkit.odfdom.doc.OdfSpreadsheetDocument;
import org.odftoolkit.odfdom.doc.table.OdfTable;
import org.odftoolkit.odfdom.doc.table.OdfTableCell;
import org.odftoolkit.odfdom.dom.OdfContentDom;
import org.odftoolkit.odfdom.dom.OdfDocumentNamespace;
import org.odftoolkit.odfdom.dom.element.number.NumberAmPmElement;
import org.odftoolkit.odfdom.dom.element.number.NumberDateStyleElement;
import org.odftoolkit.odfdom.dom.element.number.NumberDayElement;
import org.odftoolkit.odfdom.dom.element.number.NumberDayOfWeekElement;
import org.odftoolkit.odfdom.dom.element.number.NumberHoursElement;
import org.odftoolkit.odfdom.dom.element.number.NumberMinutesElement;
import org.odftoolkit.odfdom.dom.element.number.NumberMonthElement;
import org.odftoolkit.odfdom.dom.element.number.NumberNumberElement;
import org.odftoolkit.odfdom.dom.element.number.NumberScientificNumberElement;
import org.odftoolkit.odfdom.dom.element.number.NumberSecondsElement;
import org.odftoolkit.odfdom.dom.element.number.NumberTextContentElement;
import org.odftoolkit.odfdom.dom.element.number.NumberTextElement;
import org.odftoolkit.odfdom.dom.element.number.NumberYearElement;
import org.odftoolkit.odfdom.dom.element.style.StyleMapElement;
import org.odftoolkit.odfdom.dom.element.table.TableTableElement;
import org.odftoolkit.odfdom.dom.element.style.StyleTextPropertiesElement;
import org.odftoolkit.odfdom.dom.style.OdfStyleFamily;
import org.odftoolkit.odfdom.incubator.doc.office.OdfOfficeAutomaticStyles;
import org.odftoolkit.odfdom.incubator.doc.style.OdfStyle;
import org.odftoolkit.odfdom.pkg.OdfElement;
import org.odftoolkit.odfdom.pkg.OdfFileDom;

import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.Closeable;

import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.qore.jni.Hash;
import org.qore.dataprovider.format.SpreadsheetFormatCode;
import org.qore.dataprovider.format.SpreadsheetSheetName;

/**
 * Helper class for writing ODS (OpenDocument Spreadsheet) files from Qore.
 *
 * A document has one or more sheets, each with its own columns, column formats, header text, header row setting and
 * rows; the methods without a sheet index write the first sheet.
 */
public class OdsWriter implements Closeable {
    /** The state of a sheet being written */
    private static class SheetState {
        final OdfTable table;
        ArrayList<String> headers = new ArrayList<String>();
        int currentRow = 0;
        int rowOffset = 0;
        boolean headersWritten = false;
        // the header text of columns whose header is not their name, by column name
        HashMap<String, String> columnHeaders = new HashMap<String, String>();
        // false if no header row is written
        boolean writeHeaderRow = true;
        // the display format of each column by header name
        LinkedHashMap<String, SpreadsheetFormatCode> columnFormats = new LinkedHashMap<String, SpreadsheetFormatCode>();
        // the format code and cell style name of each column by position, set when the columns are known
        SpreadsheetFormatCode[] columnCodes;
        String[] columnStyles;

        SheetState(OdfTable table) {
            this.table = table;
        }
    }

    private OdfSpreadsheetDocument document;
    // an empty copy of the first table of a new document, which each added sheet is a copy of, so every sheet has the
    // layout of the first
    private TableTableElement tableTemplate;
    // the sheets in document order
    private ArrayList<SheetState> sheets = new ArrayList<SheetState>();
    // the name of the cell style of each distinct format code, so each data style is written once in the document
    private HashMap<String, String> formatStyles = new HashMap<String, String>();
    // the number of format codes written as data styles, for unique data style names
    private int dataStyleCount = 0;

    /**
     * Creates a new OdsWriter.
     *
     * @param sheet_name The name of the first sheet
     */
    public OdsWriter(String sheet_name) {
        String name = sheet_name != null && !sheet_name.isEmpty() ? sheet_name : "Sheet1";
        OdfTable table;
        try {
            document = OdfSpreadsheetDocument.newSpreadsheetDocument();
            // Remove the default empty table and create our own
            java.util.List<OdfTable> tables = document.getTableList();
            if (tables != null && !tables.isEmpty()) {
                // Rename the first table
                table = tables.get(0);
                table.setTableName(name);
            } else {
                table = OdfTable.newTable(document);
                table.setTableName(name);
            }
            tableTemplate = (TableTableElement) table.getOdfElement().cloneNode(true);
        } catch (Exception e) {
            // the document is released, as the caller gets no writer to close
            RuntimeException rv = new RuntimeException("Failed to create ODS document: " + e.getMessage(), e);
            try {
                close();
            } catch (IOException ce) {
                rv.addSuppressed(ce);
            }
            throw rv;
        }
        sheets.add(new SheetState(table));
    }

    /**
     * Creates a new OdsWriter with default settings.
     */
    public OdsWriter() {
        this("Sheet1");
    }

    /**
     * Returns why a sheet name is not accepted, or null if it is: a sheet name cannot be empty
     *
     * @param name the sheet name
     * @return the reason the name is not accepted, or null if it is valid; see {@link SpreadsheetSheetName}
     */
    public static String sheetNameIssue(String name) {
        return SpreadsheetSheetName.issue(name, false);
    }

    /**
     * Returns the key of a sheet name that two names that differ only in case share
     *
     * @param name the sheet name
     * @return the key of the name; two sheets of a document cannot have names with the same key
     */
    public static String sheetNameKey(String name) {
        return SpreadsheetSheetName.key(name);
    }

    /**
     * Adds a sheet after the others
     *
     * @param name the name of the sheet; see {@link #sheetNameIssue(String)}
     * @return the 0-based index of the sheet
     * @throws IllegalArgumentException the name is not valid, or the document already has a sheet with the name
     */
    public int addSheet(String name) {
        String issue = sheetNameIssue(name);
        if (issue != null) {
            throw new IllegalArgumentException(String.format("invalid worksheet name \"%s\": %s", name, issue));
        }
        String key = sheetNameKey(name);
        for (SheetState state : sheets) {
            if (sheetNameKey(state.table.getTableName()).equals(key)) {
                throw new IllegalArgumentException(String.format("the document already has a sheet named \"%s\"",
                    state.table.getTableName()));
            }
        }
        TableTableElement element = (TableTableElement) tableTemplate.cloneNode(true);
        sheets.get(sheets.size() - 1).table.getOdfElement().getParentNode().appendChild(element);
        OdfTable table = OdfTable.getInstance(element);
        table.setTableName(name);
        sheets.add(new SheetState(table));
        return sheets.size() - 1;
    }

    /**
     * Returns the number of sheets
     */
    public int getSheetCount() {
        return sheets.size();
    }

    /**
     * Returns the state of a sheet
     *
     * @throws IllegalArgumentException there is no sheet with the index
     */
    private SheetState getSheet(int sheet) {
        if (sheet < 0 || sheet >= sheets.size()) {
            throw new IllegalArgumentException(String.format("there is no sheet %d; the document has %d", sheet,
                sheets.size()));
        }
        return sheets.get(sheet);
    }

    /**
     * Sets the headers for the first sheet.
     */
    public void setHeaders(String[] headers) {
        ArrayList<String> list = new ArrayList<String>();
        for (String h : headers) {
            list.add(h);
        }
        setHeaders(0, list);
    }

    /**
     * Sets the headers for the first sheet.
     */
    public void setHeaders(ArrayList<String> headers) {
        setHeaders(0, headers);
    }

    /**
     * Sets the headers (the columns) of a sheet
     *
     * @param sheet the 0-based index of the sheet
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
     * Sets the header text of the columns of the first sheet whose header is not their name
     *
     * @see #setColumnHeaders(int, Hash)
     */
    public void setColumnHeaders(Hash headers) {
        setColumnHeaders(0, headers);
    }

    /**
     * Sets the header text of the columns of a sheet whose header is not their name
     *
     * @param sheet the 0-based index of the sheet
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
     * Sets the 1-based header row of the first worksheet, leaving preceding rows blank.
     * @param row the header row, from 1
     * @throws IllegalArgumentException the row is out of range, headers are disabled, or writing has started
     * @see #setHeaderRow(int, int)
     */
    public void setHeaderRow(int row) {
        setHeaderRow(0, row);
    }

    /**
     * Sets the 1-based header row of a worksheet, leaving preceding rows blank.
     * @param sheet the 0-based worksheet index
     * @param row the header row, from 1
     * @throws IllegalArgumentException the row is out of range, headers are disabled, or writing has started
     */
    public void setHeaderRow(int sheet, int row) {
        SheetState state = getSheet(sheet);
        if (state.currentRow > 0 || state.headersWritten) {
            throw new IllegalArgumentException("the header row must be set before any row is written");
        }
        if (row < 1 || row > 1048576) {
            throw new IllegalArgumentException("the header row is outside the worksheet's row range");
        }
        if (!state.writeHeaderRow && row != 1) {
            throw new IllegalArgumentException("a header row after row 1 requires headers to be enabled");
        }
        state.rowOffset = row - 1;
    }

    /**
     * Sets whether a header row is written in the first sheet
     *
     * @see #setWriteHeaders(int, boolean)
     */
    public void setWriteHeaders(boolean write) {
        setWriteHeaders(0, write);
    }

    /**
     * Sets whether a header row is written in a sheet
     *
     * @param sheet the 0-based index of the sheet
     * @param write false to write no header row: the first record is written in the first row
     * @throws IllegalArgumentException rows have already been written
     */
    public void setWriteHeaders(int sheet, boolean write) {
        SheetState state = getSheet(sheet);
        if (state.currentRow > 0) {
            throw new IllegalArgumentException("the header row must be set before any row is written");
        }
        if (!write && state.rowOffset != 0) {
            throw new IllegalArgumentException("a header row after row 1 requires headers to be enabled");
        }
        state.writeHeaderRow = write;
    }

    /**
     * Writes the header row of the first sheet, unless it is disabled with {@link #setWriteHeaders(boolean)}
     */
    public void writeHeaders() {
        writeHeaders(getSheet(0));
    }

    /**
     * Writes the header row of a sheet whose columns are known, unless it is disabled
     */
    private void writeHeaders(SheetState state) {
        if (state.headersWritten || state.headers.isEmpty()) {
            return;
        }
        if (!state.writeHeaderRow) {
            state.headersWritten = true;
            return;
        }
        for (int i = 0; i < state.headers.size(); i++) {
            OdfTableCell cell = state.table.getCellByPosition(i, state.rowOffset + state.currentRow);
            cell.setStringValue(state.columnHeaders.getOrDefault(state.headers.get(i), state.headers.get(i)));
        }
        state.currentRow++;
        state.headersWritten = true;
    }

    /**
     * Writes a data row to the first sheet
     */
    public void writeRow(Hash data) {
        writeRow(0, data);
    }

    /**
     * Writes a data row to a sheet
     *
     * Without columns, the keys of the first row written to the sheet are its columns; a column that the row has no
     * value for is an empty cell, and values that are not in a column are not written.
     *
     * @param sheet the 0-based index of the sheet
     * @param data the values of the row by column name
     */
    public void writeRow(int sheet, Hash data) {
        SheetState state = getSheet(sheet);
        // With no predefined headers, the first record's keys define the columns; they are taken before any row is
        // written, so the header row is always written first and the first record never has to be copied down a row
        // (copying lost the value types of the first record, e.g. dates became strings)
        if (state.headers.isEmpty()) {
            for (Object key : data.keySet()) {
                state.headers.add(key.toString());
            }
        }
        resolveColumnStyles(state);
        writeHeaders(state);

        for (int i = 0; i < state.headers.size(); i++) {
            Object value = data.get(state.headers.get(i));
            OdfTableCell cell = state.table.getCellByPosition(i, state.rowOffset + state.currentRow);
            SpreadsheetFormatCode code = state.columnCodes[i] != null && state.columnCodes[i].shows(value)
                ? state.columnCodes[i] : null;
            setCellValue(cell, value, code);
            if (code != null) {
                cell.getOdfElement().setTableStyleNameAttribute(state.columnStyles[i]);
            }
        }
        state.currentRow++;
    }

    /**
     * Writes the header rows of sheets without rows whose columns are known, before the document is written
     */
    private void finish() {
        for (SheetState state : sheets) {
            writeHeaders(state);
        }
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
     * Sets the display format of the columns of the first sheet
     *
     * @see #setColumnFormats(int, Hash)
     */
    public void setColumnFormats(Hash formats) {
        setColumnFormats(0, formats);
    }

    /**
     * Sets the display format of the columns of a sheet
     *
     * Each value in a column is written as its type (a number stays a number, a date a date) and shown with the
     * column's format code, which is written as an OpenDocument data style; a value that the code does not show (a
     * string in a number column, for example) is written as it is without the format.  A number in a column with a
     * percent code is written as a percentage cell, which holds the fraction and is read back as a number.  A
     * column without a format code is written as before.
     *
     * @param sheet the 0-based index of the sheet
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
     * Resolves the format code and cell style of each column of a sheet once its columns are known
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
        String[] styles = new String[state.headers.size()];
        for (int i = 0; i < state.headers.size(); ++i) {
            SpreadsheetFormatCode code = state.columnFormats.get(state.headers.get(i));
            if (code == null) {
                continue;
            }
            String style = formatStyles.get(code.getCode());
            if (style == null) {
                style = createCellStyle(code);
                formatStyles.put(code.getCode(), style);
            }
            codes[i] = code;
            styles[i] = style;
        }
        state.columnCodes = codes;
        state.columnStyles = styles;
    }

    /**
     * Creates the data style of a format code and a cell style using it, and returns the name of the cell style
     *
     * The data style is built from the parsed format code rather than with odfdom's format string parsers, which
     * do not support sections and loop forever on some invalid codes.  A code with more than one section is a data
     * style per section: the last one (negative, or zero with three sections) is the one the cell style uses, with
     * style:map conditions selecting the others, as LibreOffice writes them.
     */
    private String createCellStyle(SpreadsheetFormatCode code) {
        OdfContentDom dom;
        try {
            dom = document.getContentDom();
        } catch (Exception e) {
            throw new RuntimeException("Failed to get the ODS content: " + e.getMessage(), e);
        }
        OdfOfficeAutomaticStyles auto_styles = dom.getOrCreateAutomaticStyles();
        String name = "QoreFmt" + (++dataStyleCount);

        List<SpreadsheetFormatCode.Section> sections = code.getSections();
        int last = sections.size() - 1;
        ArrayList<String> section_names = new ArrayList<String>();
        for (int i = 0; i < last; ++i) {
            String section_name = name + "P" + i;
            createDataStyle(dom, auto_styles, code.getKind(), sections.get(i), section_name);
            section_names.add(section_name);
        }
        OdfElement main = createDataStyle(dom, auto_styles, code.getKind(), sections.get(last), name);
        if (sections.size() == 2) {
            addMap(dom, main, "value()>=0", section_names.get(0));
        } else if (sections.size() == 3) {
            addMap(dom, main, "value()>0", section_names.get(0));
            addMap(dom, main, "value()<0", section_names.get(1));
        }

        OdfStyle cell_style = auto_styles.newStyle(OdfStyleFamily.TableCell);
        cell_style.setStyleDataStyleNameAttribute(name);
        return cell_style.getStyleNameAttribute();
    }

    /** Creates the data style of one section of a format code */
    private static OdfElement createDataStyle(OdfFileDom dom, OdfOfficeAutomaticStyles auto_styles,
            SpreadsheetFormatCode.Kind kind, SpreadsheetFormatCode.Section section, String name) {
        OdfElement style;
        switch (kind) {
            case NUMBER:
                style = auto_styles.newNumberNumberStyleElement(name);
                break;
            case PERCENT:
                style = auto_styles.newNumberPercentageStyleElement(name);
                break;
            case DATE: {
                NumberDateStyleElement date_style = auto_styles.newNumberDateStyleElement(name);
                // the parts are shown as given, not as the language would show them
                date_style.setNumberFormatSourceAttribute("fixed");
                style = date_style;
                break;
            }
            default:
                style = auto_styles.newNumberTextStyleElement(name);
                break;
        }
        if (section.color != null) {
            StyleTextPropertiesElement props = new StyleTextPropertiesElement(dom);
            props.setFoColorAttribute(SpreadsheetFormatCode.COLORS.get(section.color));
            style.appendChild(props);
        }
        for (SpreadsheetFormatCode.Part part : section.parts) {
            style.appendChild(createPart(dom, part));
        }
        return style;
    }

    /** Creates the data style element of a part of a format code section */
    private static OdfElement createPart(OdfFileDom dom, SpreadsheetFormatCode.Part part) {
        if (part instanceof SpreadsheetFormatCode.Literal) {
            NumberTextElement text = new NumberTextElement(dom);
            text.setTextContent(((SpreadsheetFormatCode.Literal) part).text);
            return text;
        }
        if (part instanceof SpreadsheetFormatCode.TextValue) {
            return new NumberTextContentElement(dom);
        }
        if (part instanceof SpreadsheetFormatCode.Digits) {
            SpreadsheetFormatCode.Digits digits = (SpreadsheetFormatCode.Digits) part;
            if (digits.scientific) {
                NumberScientificNumberElement number = new NumberScientificNumberElement(dom);
                number.setNumberDecimalPlacesAttribute(digits.decimalPlaces);
                number.setNumberMinIntegerDigitsAttribute(digits.minIntegerDigits);
                number.setNumberMinExponentDigitsAttribute(digits.minExponentDigits);
                if (digits.grouping) {
                    number.setNumberGroupingAttribute(true);
                }
                setNumberAttribute(number, "min-decimal-places", digits.minDecimalPlaces);
                return number;
            }
            NumberNumberElement number = new NumberNumberElement(dom);
            number.setNumberDecimalPlacesAttribute(digits.decimalPlaces);
            number.setNumberMinIntegerDigitsAttribute(digits.minIntegerDigits);
            if (digits.grouping) {
                number.setNumberGroupingAttribute(true);
            }
            if (digits.thousandsScale > 0) {
                number.setNumberDisplayFactorAttribute(Math.pow(1000, digits.thousandsScale));
            }
            setNumberAttribute(number, "min-decimal-places", digits.minDecimalPlaces);
            return number;
        }
        SpreadsheetFormatCode.DatePart date_part = (SpreadsheetFormatCode.DatePart) part;
        OdfElement element;
        switch (date_part.field) {
            case YEAR:
                element = new NumberYearElement(dom);
                break;
            case MONTH:
                element = new NumberMonthElement(dom);
                if (date_part.textual) {
                    ((NumberMonthElement) element).setNumberTextualAttribute(true);
                }
                break;
            case DAY:
                element = new NumberDayElement(dom);
                break;
            case DAY_OF_WEEK:
                element = new NumberDayOfWeekElement(dom);
                break;
            case HOURS:
                element = new NumberHoursElement(dom);
                break;
            case MINUTES:
                element = new NumberMinutesElement(dom);
                break;
            case SECONDS:
                element = new NumberSecondsElement(dom);
                if (date_part.decimalPlaces > 0) {
                    ((NumberSecondsElement) element).setNumberDecimalPlacesAttribute(date_part.decimalPlaces);
                }
                break;
            default:
                return new NumberAmPmElement(dom);
        }
        element.setAttributeNS(OdfDocumentNamespace.NUMBER.getUri(), "number:style",
            date_part.longForm ? "long" : "short");
        return element;
    }

    /** Sets a number attribute that odfdom has no setter for */
    private static void setNumberAttribute(OdfElement element, String name, int value) {
        element.setAttributeNS(OdfDocumentNamespace.NUMBER.getUri(), "number:" + name, String.valueOf(value));
    }

    /** Adds a style:map selecting the data style of another section */
    private static void addMap(OdfFileDom dom, OdfElement style, String condition, String apply_style) {
        StyleMapElement map = new StyleMapElement(dom);
        map.setStyleConditionAttribute(condition);
        map.setStyleApplyStyleNameAttribute(apply_style);
        style.appendChild(map);
    }

    /**
     * Sets the cell value based on the object type.
     */
    private void setCellValue(OdfTableCell cell, Object value, SpreadsheetFormatCode code) {
        if (value == null) {
            // leave the cell empty so that it is read back as no value rather than as an empty string
            return;
        } else if (value instanceof Boolean) {
            cell.setBooleanValue((Boolean) value);
        } else if (value instanceof Number) {
            double number = ((Number) value).doubleValue();
            if (code != null && code.getKind() == SpreadsheetFormatCode.Kind.PERCENT) {
                // a percentage cell holds the fraction (0.25 for 25%) and is read back as a number
                cell.setPercentageValue(number);
                cell.setDisplayText(String.valueOf(number));
            } else {
                cell.setDoubleValue(number);
            }
        } else if (value instanceof ZonedDateTime) {
            setDateTimeValue(cell, ((ZonedDateTime) value).toLocalDateTime());
        } else if (value instanceof Date) {
            setDateTimeValue(cell, LocalDateTime.ofInstant(((Date) value).toInstant(), ZoneId.systemDefault()));
        } else {
            cell.setStringValue(value.toString());
        }
    }

    /**
     * Sets a date cell to a date and time: the clock time of the value, as office:date-value has no zone
     *
     * odfdom's setDateValue() writes only the date, and setLocalDateTimeValue() writes LocalDateTime.toString(),
     * which leaves out zero seconds and is then not a valid xsd:dateTime, so the value is set with the seconds
     * always written
     */
    private static void setDateTimeValue(OdfTableCell cell, LocalDateTime datetime) {
        String value = datetime.format(DateTimeFormatter.ISO_LOCAL_DATE_TIME);
        cell.setLocalDateTimeValue(datetime);
        cell.getOdfElement().setOfficeDateValueAttribute(value);
        cell.setDisplayText(value);
    }

    /**
     * Writes the document to a file.
     */
    public void writeToFile(String path) throws Exception {
        finish();
        document.save(path);
    }

    /**
     * Writes the document to an output stream.
     */
    public void writeToStream(OutputStream stream) throws Exception {
        finish();
        document.save(stream);
    }

    /**
     * Writes the document to a Qore output stream.
     */
    public void writeToStream(qore.Qore.OutputStream stream) throws Exception {
        writeToStream(new org.qore.jni.QoreOutputStreamWrapper(stream));
    }

    /**
     * Returns the document contents as bytes.
     */
    public byte[] getBytes() throws Exception {
        finish();
        try (ByteArrayOutputStream out = new ByteArrayOutputStream()) {
            document.save(out);
            return out.toByteArray();
        }
    }

    /**
     * Gets the current row count of the first sheet (including header if written).
     */
    public int getRowCount() {
        return getRowCount(0);
    }

    /**
     * Gets the current row count of a sheet (including header if written).
     *
     * @param sheet the 0-based index of the sheet
     */
    public int getRowCount(int sheet) {
        return getSheet(sheet).currentRow;
    }

    @Override
    public void close() throws IOException {
        if (document != null) {
            document.close();
            document = null;
        }
    }
}
