// Copyright (C) 2026 Qore Technologies, s.r.o.
// SPDX-License-Identifier: MIT
package org.qore.jni.test;

import java.sql.Time;
import java.sql.Timestamp;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.MonthDay;
import java.time.OffsetDateTime;
import java.time.OffsetTime;
import java.time.Period;
import java.time.Year;
import java.time.YearMonth;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;

/** Values of the Java date and time classes, and methods taking them, for the conversion tests */
public final class JavaTimeTypes {
    /** A field of a date and time class, set and read from Qore */
    public static OffsetDateTime when;

    private JavaTimeTypes() {
    }

    public static OffsetDateTime offsetDateTime() {
        return OffsetDateTime.of(2026, 10, 21, 10, 15, 30, 123456000, ZoneOffset.ofHoursMinutes(5, 30));
    }

    public static OffsetTime offsetTime() {
        return OffsetTime.of(10, 15, 30, 500000000, ZoneOffset.ofHours(-3));
    }

    public static LocalDate localDate() {
        return LocalDate.of(2026, 10, 21);
    }

    public static LocalTime localTime() {
        // the seconds are 0, which LocalTime.toString() leaves out
        return LocalTime.of(10, 15);
    }

    public static LocalDateTime localDateTime() {
        return LocalDateTime.of(2026, 10, 21, 10, 15, 30);
    }

    public static ZonedDateTime zonedDateTime() {
        return ZonedDateTime.of(2026, 10, 21, 10, 15, 30, 0, ZoneId.of("America/New_York"));
    }

    public static Instant instant() {
        return Instant.parse("2026-10-21T10:15:30Z");
    }

    public static Timestamp timestamp() {
        return Timestamp.valueOf("2026-10-21 10:15:30.25");
    }

    public static java.sql.Date sqlDate() {
        return java.sql.Date.valueOf("2026-10-21");
    }

    public static Time sqlTime() {
        return Time.valueOf("10:15:30");
    }

    // classes that are not converted
    public static Duration duration() {
        return Duration.ofMillis(1500);
    }

    public static Period period() {
        return Period.of(1, 2, 3);
    }

    public static Year year() {
        return Year.of(2026);
    }

    public static YearMonth yearMonth() {
        return YearMonth.of(2026, 10);
    }

    public static MonthDay monthDay() {
        return MonthDay.of(10, 21);
    }

    // the class and value received for each class
    public static String takeOffsetDateTime(OffsetDateTime v) {
        return describe(v);
    }

    public static String takeOffsetTime(OffsetTime v) {
        return describe(v);
    }

    public static String takeLocalDate(LocalDate v) {
        return describe(v);
    }

    public static String takeLocalTime(LocalTime v) {
        return describe(v);
    }

    public static String takeLocalDateTime(LocalDateTime v) {
        return describe(v);
    }

    public static String takeZonedDateTime(ZonedDateTime v) {
        return describe(v);
    }

    public static String takeInstant(Instant v) {
        return describe(v);
    }

    public static String takeTimestamp(Timestamp v) {
        return describe(v);
    }

    public static String takeSqlDate(java.sql.Date v) {
        return describe(v);
    }

    public static String takeSqlTime(Time v) {
        return describe(v);
    }

    public static String takeDuration(Duration v) {
        return describe(v);
    }

    public static String whenString() {
        return describe(when);
    }

    private static String describe(Object v) {
        return v == null ? "null" : v.getClass().getName() + " " + v;
    }
}
