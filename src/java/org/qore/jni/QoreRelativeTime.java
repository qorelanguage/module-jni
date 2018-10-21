package org.qore.jni;

//! class representing a Qore relative time value (duration)
/**
 */
public class QoreRelativeTime {
    public int year;    // year
    public int month;   // month
    public int day;     // day
    public int hour;    // hours
    public int minute;  // minutes
    public int second;  // seconds
    public int us;      // microseconds

    public QoreRelativeTime(int year, int month, int day, int hour, int minute, int second, int us) {
        this.year = year;
        this.month = month;
        this.day = day;
        this.hour = hour;
        this.minute = minute;
        this.second = second;
        this.us = us;
    }
 }
