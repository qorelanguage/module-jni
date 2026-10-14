# -*- mode: qore; indent-tabs-mode: nil -*-
# Qore OdsWriteDataProvider class definition

/*  OdsWriteDataProvider.qc Copyright 2026 Qore Technologies, s.r.o.

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

#! the OdsDataProvider namespace
public namespace OdsDataProvider {
#! Provides a data provider for writing ODS files
public class OdsWriteDataProvider inherits DataProvider::AbstractDataProvider {
    public {
        #! Provider info
        const ProviderInfo = <DataProviderInfo>{
            "type": "OdsWriteDataProvider",
            "supports_create": True,
            "has_record": True,
            "constructor_options": ConstructorOptions,
        };

        #! Constructor options
        const ConstructorOptions = {
            "path": <DataProviderOptionInfo>{
                "display_name": "ODS Output Path",
                "short_desc": "The location to write the ODS file",
                "type": AbstractDataProviderType::get(StringType),
                "desc": "The location for the output ODS data file; mutually exclusive with `stream`.\n\n"
                    "The output location is handled by a call to "
                    "[FileLocationHandler::getOutputStreamForLocation()](https://qoretechnologies.com/"
                    "manual/qorus/current/qore/modules/FileLocationHandler/html/"
                    "class_file_location_handler_1_1_file_location_handler.html"
                    "#a46f7e9cc0e76e0d7e30cae79513cd35)",
                "example_value": "file:///path/to/output.ods",
            },
            "stream": <DataProviderOptionInfo>{
                "display_name": "ODS Output Stream",
                "short_desc": "An output stream for ODS data",
                "type": AbstractDataProviderType::get(new Type("OutputStream")),
                "desc": "An output stream for ODS data; mutually exclusive with `path`",
            },
            "worksheet": <DataProviderOptionInfo>{
                "display_name": "Worksheet Name",
                "short_desc": "The name of the worksheet to create",
                "type": AbstractDataProviderType::get(StringType),
                "desc": "The name of the worksheet to create; if not provided, defaults to `Sheet1`",
                "example_value": "Sheet1",
            },
            "headers": <DataProviderOptionInfo>{
                "display_name": "Column Headers",
                "short_desc": "List of header / column names for the data",
                "type": AbstractDataProviderType::get(new Type("softlist<string>")),
                "desc": "List of header / column names for the data; if not provided, headers will be "
                    "auto-detected from the first record",
            },
            "column_formats": <DataProviderOptionInfo>{
                "display_name": "Column Display Formats",
                "short_desc": "The display format of each column, such as dd.mm.yyyy or #,##0.00",
                "type": AbstractDataProviderType::get(new Type("hash<string, string>")),
                "desc": "The display format of columns by their header name, so that whoever opens the file sees "
                    "each value as intended: a date as `dd.mm.yyyy`, an amount as `#,##0.00`, or a share as "
                    "`0.0%`.  The cells keep their real values (a date stays a date and a number a number); only "
                    "the way they are shown changes.  Each format is written as an OpenDocument data style, which "
                    "LibreOffice and Excel show the same way.  A column without a format is written as without "
                    "this option.\n\n"
                    "Format codes are written as in LibreOffice and Excel:\n"
                    "- **numbers**: `0` (a digit that is always shown), `#` (a digit shown when significant), `,` "
                    "between digits (thousands grouping), `,` after the last digit (divide by 1000), `.` (the "
                    "decimal point), and `E+00` (scientific notation); up to three sections for positive, negative "
                    "and zero values separated by `;`, such as `#,##0.00;(#,##0.00)` or `#,##0.00;#,##0.00-`\n"
                    "- **percentages**: a `%` in each section, such as `0.0%`; the value is a fraction, so `0.25` "
                    "is shown as `25.0%`, and it is written as a percentage cell, which is read back as a number\n"
                    "- **dates and times**: `yy` or `yyyy` (year), `m`, `mm`, `mmm` or `mmmm` (month as a number "
                    "or a name), `d` or `dd` (day), `ddd` or `dddd` (weekday name), `h` or `hh` (hour), `m` or "
                    "`mm` directly after an hour or before a second (minute), `s` or `ss` (second), `.0` to "
                    "`.000` after the second (fractions of a second), and `AM/PM`, such as `dd.mm.yyyy hh:mm`\n"
                    "- **text**: `@`\n\n"
                    "Text in a code is written in double quotes (`\"EUR\"`), escaped with a backslash (`\\x`), as "
                    "a currency symbol in brackets (`[$€-407]`), or as itself for `$ - + / ( ) :`, spaces and "
                    "symbols such as `€`; a section can start with a color such as `[Red]`.\n\n"
                    "A value that its column's code does not show, such as a string in a number column, is "
                    "written without the format.  An invalid or unsupported code (`General`, fractions, fill "
                    "characters, conditions, or elapsed times), or a column that is not written, raises "
                    "`ODS-WRITE-OPTION-ERROR` naming the column and the problem",
                "example_value": {"Date": "dd.mm.yyyy", "Amount": "#,##0.00;(#,##0.00)", "Share": "0.0%"},
            },
            "column_masks": <DataProviderOptionInfo>{
                "display_name": "Column Masks",
                "short_desc": "The display format of each column as a mask, such as DD.MM.YYYY or #.##0,00 €",
                "type": AbstractDataProviderType::get(new Type("hash<string, string>")),
                "desc": "The display format of columns by column name as masks of the "
                    "[ValueMask](https://qoretechnologies.com/manual/qorus/current/qore/modules/ValueMask/html/"
                    "index.html) module, the masks that CSV files and sheet "
                    "contracts are read and written with: a date as `DD.MM.YYYY`, a time as `hh:mm AM`, an amount "
                    "as `#.##0,00 €` or a share as `0.#%`.  Each mask becomes the display format code of its column "
                    "(`DD.MM.YYYY` is `dd\\.mm\\.yyyy`, `#.##0,00 €` is `#,##0.00\" €\"`), so the cells keep their "
                    "real values (a date stays a date and a number a number) and are shown as the mask describes, "
                    "with these differences from text:\n"
                    "- the group separator and decimal mark are shown as the language of whoever opens the file "
                    "uses them; the mask's separators only say whether digits are grouped and how many decimals "
                    "are shown\n"
                    "- `Mon` shows the month name in the language of whoever opens the file\n"
                    "- a final `#` shows at most 10 decimals, and a whole number with the decimal mark after it "
                    "(`12.`)\n\n"
                    "A mask without a display format raises `ODS-WRITE-OPTION-ERROR` naming the column and the "
                    "reason: `ZZ` (a cell holds no UTC offset), `MON` (there is no upper-case month name), `sss` "
                    "not directly after `SS.`, a minute that is not directly after an hour or before the seconds, "
                    "and a numeric month that is.  A column cannot have both a mask and a format code in "
                    "`column_formats`",
                "example_value": {"Date": "DD.MM.YYYY", "Amount": "#.##0,00 €", "Share": "0.#%"},
            },
            "column_headers": <DataProviderOptionInfo>{
                "display_name": "Column Header Text",
                "short_desc": "The header text of each column whose header is not its name",
                "type": AbstractDataProviderType::get(new Type("hash<string, string>")),
                "desc": "The header text of columns by column name, for columns whose header is not their name.  "
                    "The records are written by column name, so `headers` and the other column options name the "
                    "columns as the records do; this option only changes the text of the header row",
                "example_value": {"order_id": "Order No.", "amount": "Amount (EUR)"},
            },
            "write_headers": <DataProviderOptionInfo>{
                "display_name": "Write Header Row",
                "short_desc": "Write the column headers as the first row",
                "type": AbstractDataProviderType::get(SoftBoolType),
                "desc": "Set to `false` to write no header row: the first record is written in the first row of "
                    "the worksheet; the default is `true`",
                "default_value": True,
            },
            "timezone": <DataProviderOptionInfo>{
                "display_name": "Time Zone",
                "short_desc": "The time zone whose wall-clock time dates are written in",
                "type": AbstractDataProviderType::get(StringType),
                "desc": "The time zone region (such as `Europe/Prague` or `America/New_York`) whose wall-clock "
                    "time dates are written in, as a cell holds a date and time without a time zone; if not given, "
                    "a date is written as its own wall-clock time",
                "example_value": "Europe/Prague",
            },
        };
    }

    private {
        #! The output path
        *string path;

        #! The output stream
        *OutputStream stream;

        #! The name of the object
        string name;

        #! The ODS writer object
        OdsWriter writer;

        #! The options
        *hash<auto> opts;

        #! Records created count
        int created = 0;

        #! The time zone whose wall-clock time dates are written in, if any
        *TimeZone timezone;
    }

    #! Creates the OdsWriteDataProvider with the output path and optionally an option hash
    constructor(string path, *hash<auto> opts) {
        *hash<auto> copts = checkOptions("ODS-WRITE-OPTION-ERROR", ConstructorOptions, opts);
        if (copts.stream) {
            error("ODS-WRITE-OPTION-ERROR", "cannot provide \"stream\" option when path is given as first "
                "argument; use either one or the other");
        }
        if (copts.path) {
            error("ODS-WRITE-OPTION-ERROR", "cannot provide \"path\" option when path is given as first "
                "argument");
        }
        self.path = path;
        name = path;
        self.opts = copts - ("path", "stream");
        setupWriter();
    }

    #! Creates the OdsWriteDataProvider with an output stream and optionally an option hash
    constructor(OutputStream stream, *hash<auto> opts) {
        *hash<auto> copts = checkOptions("ODS-WRITE-OPTION-ERROR", ConstructorOptions, opts);
        if (copts.path) {
            error("ODS-WRITE-OPTION-ERROR", "cannot provide \"path\" option when stream is given as first "
                "argument; use either one or the other");
        }
        if (copts.stream) {
            error("ODS-WRITE-OPTION-ERROR", "cannot provide \"stream\" option when stream is given as first "
                "argument");
        }
        self.stream = stream;
        name = sprintf("ods-stream=%s", stream.uniqueHash());
        self.opts = copts - ("path", "stream");
        setupWriter();
    }

    #! Creates the object from constructor options
    constructor(*hash<auto> options) {
        *hash<auto> copts = checkOptions("ODS-WRITE-OPTION-ERROR", ConstructorOptions, options);
        if (copts.path) {
            if (copts.stream) {
                error("ODS-WRITE-OPTION-ERROR", "cannot provide both \"stream\" and \"path\" options; use either "
                    "one or the other");
            }
            path = copts.path;
            name = path;
        } else if (copts.stream) {
            stream = copts.stream;
            name = sprintf("ods-stream=%s", stream.uniqueHash());
        } else {
            name = "ods-data";
        }
        self.opts = copts - ("path", "stream");
        setupWriter();
    }

    #! Sets up the writer
    /** @throw ODS-WRITE-OPTION-ERROR an invalid format code in \c column_formats, a mask without a display format
        in \c column_masks, a column with both, an invalid time zone, or with \c headers, a column option for a
        column that is not written
    */
    private setupWriter() {
        hash<string, string> formats = {};
        if (opts.column_formats) {
            checkColumnFormats();
            formats = opts.column_formats;
        }
        if (opts.column_masks) {
            formats += getMaskFormats();
        }
        if (opts.timezone) {
            try {
                timezone = new TimeZone(opts.timezone);
            } catch (hash<ExceptionInfo> ex) {
                error("ODS-WRITE-OPTION-ERROR", "invalid \"timezone\" option %y: %s: %s", opts.timezone, ex.err,
                    ex.desc);
            }
        }
        if (opts.headers) {
            checkColumns(opts.headers);
        }
        writer = new OdsWriter(opts.worksheet ?? "Sheet1");
        if (opts.headers) {
            writer.setHeaders(opts.headers);
        }
        if (formats) {
            writer.setColumnFormats(formats);
        }
        if (opts.column_headers) {
            writer.setColumnHeaders(opts.column_headers);
        }
        if (exists opts.write_headers && !opts.write_headers) {
            writer.setWriteHeaders(False);
        }
    }

    #! Checks the \c column_formats option: each format code is valid
    /** @throw ODS-WRITE-OPTION-ERROR an invalid format code
    */
    private checkColumnFormats() {
        foreach hash<auto> i in (opts.column_formats.pairIterator()) {
            *string issue = OdsWriter::formatCodeIssue(i.value);
            if (issue) {
                error("ODS-WRITE-OPTION-ERROR", "invalid \"column_formats\" option: column %y: %s", i.key, issue);
            }
        }
    }

    #! Returns the display format codes of the \c column_masks option
    /** @throw ODS-WRITE-OPTION-ERROR a mask without a display format, or a column with both a mask and a format
        code
    */
    private hash<string, string> getMaskFormats() {
        hash<string, string> rv = {};
        foreach hash<auto> i in (opts.column_masks.pairIterator()) {
            if (exists opts.column_formats{i.key}) {
                error("ODS-WRITE-OPTION-ERROR", "invalid \"column_masks\" option: column %y also has a format code "
                    "in the \"column_formats\" option; a column can have one or the other", i.key);
            }
            if (*string issue = ValueMask::spreadsheetFormatIssue(i.value)) {
                error("ODS-WRITE-OPTION-ERROR", "invalid \"column_masks\" option: column %y: %s", i.key, issue);
            }
            rv{i.key} = ValueMask::toSpreadsheetFormat(i.value);
        }
        return rv;
    }

    #! Checks that each column in the \c column_formats, \c column_masks and \c column_headers options is written
    /** @param columns the columns written
        @throw ODS-WRITE-OPTION-ERROR a column option for a column that is not written
    */
    private checkColumns(*list<string> columns) {
        hash<string, bool> written = map {$1: True}, columns;
        foreach hash<auto> i in ({
                "column_formats": "a format",
                "column_masks": "a mask",
                "column_headers": "a header",
            }.pairIterator()) {
            *list<string> unknown = select keys opts{i.key}, !written{$1};
            if (unknown) {
                error("ODS-WRITE-OPTION-ERROR", "invalid \"%s\" option: %s is given for %s %y, which %s not "
                    "written; the columns written are: %y", i.key, i.value, unknown.size() == 1 ? "column" : "columns",
                    unknown.size() == 1 ? unknown[0] : unknown, unknown.size() == 1 ? "is" : "are", columns);
            }
        }
    }

    #! Returns the object name
    string getName() {
        return name;
    }

    #! Destructor - ensures file is written
    destructor() {
        commit();
    }

    #! Creates a single record in the data provider
    /** @throw ODS-WRITE-OPTION-ERROR without \c headers, the first record has no key for a column in
        \c column_formats
    */
    private *hash<auto> createRecordImpl(hash<auto> rec, *hash<auto> create_options) {
        # without headers, the first record's keys are the columns
        if (!created && !opts.headers && (opts.column_formats || opts.column_masks || opts.column_headers)) {
            checkColumns(keys rec);
        }
        hash<auto> row = rec;
        # dates as the wall-clock time in the time zone
        if (timezone) {
            row = map {$1.key: $1.value.typeCode() == NT_DATE ? timezone.date($1.value) : $1.value},
                rec.pairIterator();
        }
        writer.writeRow(row);
        ++created;
        return rec;
    }

    #! Commits (writes) the ODS file
    commit() {
        if (!writer) {
            return;
        }
        on_exit {
            writer.close();
            delete writer;
        }
        if (path) {
            OutputStreamWrapper os = FileLocationHandler::getOutputStreamForLocation(path);
            on_exit {
                os.close();
                os.waitForIo();
            }
            writer.writeToStream(os);
        } else if (stream) {
            writer.writeToStream(stream);
        }
    }

    #! Completes the create operations: writes the ODS data to its path or stream, if any
    /** @return the generated ODS data and the number of records in it, if there is no output path or stream;
        otherwise no value is returned
    */
    private *hash<GeneratedDataInfo> finishCreateImpl() {
        if (path || stream) {
            commit();
            return;
        }
        # the data must be read before commit() closes the writer
        on_exit commit();
        return <GeneratedDataInfo>{
            "data": getData(),
            "record_count": created,
        };
    }

    #! Returns the data as binary
    binary getData() {
        if (!writer) {
            throw "ODS-WRITE-ERROR", "writer has already been committed";
        }
        return writer.getBytes();
    }

    #! Returns the number of records created
    int getCreatedCount() {
        return created;
    }

    #! Returns the record type: the \c headers columns, or an open record type if headers are taken from the first record
    /** @param search_options ignored

        @return a field for each column in the \c headers option, or @ref NOTHING (an open record shape) if no
        headers are given, in which case the first record's keys are used as the columns
    */
    private *hash<string, AbstractDataField> getRecordTypeImpl(*hash<auto> search_options) {
        return map {
            $1: new QoreDataField($1, "ODS column " + $1, AbstractDataProviderType::anyDataType)
        }, opts.headers;
    }

    #! Returns data provider static info
    private hash<DataProviderInfo> getStaticInfoImpl() {
        return ProviderInfo;
    }
}
} # OdsDataProvider namespace
