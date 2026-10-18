/*  SpreadsheetSheetName.java Copyright 2026 Qore Technologies, s.r.o.

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

/**
 * The rules for the names of the worksheets of a written workbook
 *
 * A worksheet name is not empty, and two worksheets of a workbook cannot have names that differ only in case.  An
 * Excel worksheet name also has at most 31 characters, contains none of {@code : \ / ? * [ ]}, and does not begin
 * or end with an apostrophe, as Apache POI checks them; an OpenDocument table name has no other rules.
 */
public final class SpreadsheetSheetName {
    /** The maximum length of an Excel worksheet name */
    public static final int EXCEL_MAX_LENGTH = 31;

    /** The characters that an Excel worksheet name cannot contain */
    private static final String INVALID_CHARS = ":\\/?*[]";

    private SpreadsheetSheetName() {
    }

    /**
     * Returns why a worksheet name is not accepted, or null if it is
     *
     * @param name the worksheet name
     * @param excel true for the rules of an Excel worksheet name, false for an OpenDocument table name
     * @return the reason the name is not accepted, or null if it is valid
     */
    public static String issue(String name, boolean excel) {
        if (name == null || name.isEmpty()) {
            return "a worksheet name cannot be empty";
        }
        if (!excel) {
            return null;
        }
        // UTF-16 characters, as Apache POI counts them
        if (name.length() > EXCEL_MAX_LENGTH) {
            return String.format("a worksheet name can have at most %d characters; this one has %d",
                EXCEL_MAX_LENGTH, name.length());
        }
        for (int i = 0; i < name.length(); ++i) {
            char c = name.charAt(i);
            if (INVALID_CHARS.indexOf(c) >= 0) {
                return String.format("it contains \"%c\"; a worksheet name cannot contain any of : \\ / ? * [ ]", c);
            }
        }
        if (name.charAt(0) == '\'' || name.charAt(name.length() - 1) == '\'') {
            return "a worksheet name cannot begin or end with an apostrophe (')";
        }
        return null;
    }

    /**
     * Returns the key of a worksheet name that two names that differ only in case share
     *
     * The characters are compared as {@link String#equalsIgnoreCase(String)} compares them, as Apache POI compares
     * the names of the worksheets of a workbook.
     *
     * @param name the worksheet name
     * @return the name with each character in the same case
     */
    public static String key(String name) {
        StringBuilder key = new StringBuilder(name.length());
        for (int i = 0; i < name.length(); ++i) {
            key.append(Character.toLowerCase(Character.toUpperCase(name.charAt(i))));
        }
        return key.toString();
    }
}
