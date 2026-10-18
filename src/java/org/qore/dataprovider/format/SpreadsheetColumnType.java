/*  SpreadsheetColumnType.java Copyright 2026 Qore Technologies, s.r.o.

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


package org.qore.dataprovider.format;

import java.math.BigDecimal;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;

/**
 * The types of the columns of a spreadsheet read with types, and the conversion of cell values to them
 *
 * Cell values are read as a String, a Double, a Boolean, or a ZonedDateTime (dates and times); a column type converts
 * them as follows:
 * - "string": text; numbers, booleans and dates are converted to text (a whole number without a decimal point, a
 *   date in ISO-8601 format)
 * - "int": numbers without a fraction, as a Long
 * - "float": numbers
 * - "number": numbers, as an arbitrary-precision BigDecimal
 * - "bool": booleans
 * - "date": dates and times
 * - "any": every value as read
 */
public final class SpreadsheetColumnType {
    /** The column types, as listed in messages */
    public static final String TYPE_LIST = "string, int, float, number, bool, date, any";

    /** Returned by convert() for a value that the type does not accept */
    public static final Object MISMATCH = new Object();

    private SpreadsheetColumnType() {
    }

    /**
     * Returns true if the string is a column type
     */
    public static boolean isType(String type) {
        if (type == null) {
            return false;
        }
        switch (type) {
            case "string":
            case "int":
            case "float":
            case "number":
            case "bool":
            case "date":
            case "any":
                return true;
            default:
                return false;
        }
    }

    /**
     * Converts a cell value to a column type
     *
     * @param val the value; not null
     * @param type the column type
     *
     * @return the converted value, or MISMATCH if the type does not accept the value
     */
    public static Object convert(Object val, String type) {
        switch (type) {
            case "any":
                return val;
            case "string":
                if (val instanceof String) {
                    return val;
                }
                return toText(val);
            case "int":
                if (val instanceof Double) {
                    double d = (Double)val;
                    if (d == Math.rint(d) && !Double.isInfinite(d) && Math.abs(d) <= 9007199254740992.0) {
                        return (long)d;
                    }
                } else if (val instanceof Long || val instanceof Integer) {
                    return ((Number)val).longValue();
                }
                return MISMATCH;
            case "float":
                if (val instanceof Number) {
                    return ((Number)val).doubleValue();
                }
                return MISMATCH;
            case "number":
                if (val instanceof Double) {
                    return BigDecimal.valueOf((Double)val);
                } else if (val instanceof Long || val instanceof Integer) {
                    return BigDecimal.valueOf(((Number)val).longValue());
                }
                return MISMATCH;
            case "bool":
                return val instanceof Boolean ? val : MISMATCH;
            case "date":
                return val instanceof ZonedDateTime ? val : MISMATCH;
            default:
                throw new IllegalArgumentException("unknown column type: " + type);
        }
    }

    /**
     * Returns a value as text: a whole number without a decimal point, a date in ISO-8601 format
     */
    public static String toText(Object val) {
        if (val instanceof Double) {
            double d = (Double)val;
            if (Double.isNaN(d) || Double.isInfinite(d)) {
                return Double.toString(d);
            }
            return BigDecimal.valueOf(d).stripTrailingZeros().toPlainString();
        }
        if (val instanceof ZonedDateTime) {
            return ((ZonedDateTime)val).toLocalDateTime().format(DateTimeFormatter.ISO_LOCAL_DATE_TIME);
        }
        return val.toString();
    }

    /**
     * Describes a value for a message: text is quoted
     */
    public static String describe(Object val) {
        if (val instanceof String) {
            return "\"" + val + "\"";
        }
        return toText(val);
    }
}
