/** Java wrapper for the %Qore HTTPClient class
 *
 */
package org.qore.jni;

// java imports
import java.util.Map;
import java.util.LinkedHashMap;
import java.util.ArrayList;
import java.time.ZonedDateTime;
import java.math.BigDecimal;

// qore jni imports
import org.qore.jni.QoreRelativeTime;
import org.qore.jni.QoreClosureMarker;

//! Java Hash class to make it easier to work with %Qore hash data
/**
 */
public class Hash extends LinkedHashMap<String, Object> {
    //! Creates the object
    public Hash() {
        super();
    }

    //! Creates the object
    public Hash(Map<String, Object> m) {
        super(m);
    }

    //! Returns the given key as a boolean
    public boolean getBool(String key) {
        return (Boolean)get(key);
    }

    //! Returns the given key as an int
    public int getByte(String key) {
        return (Integer)get(key);
    }

    //! Returns the given key as a char
    public char getChar(String key) {
        return (Character)get(key);
    }

    //! Returns the given key as a short
    public short getShort(String key) {
        return (Short)get(key);
    }

    //! Returns the given key as an int
    public int getInt(String key) {
        return (Integer)get(key);
    }

    //! Returns the given key as a long
    public long getLong(String key) {
        return (Long)get(key);
    }

    //! Returns the given key as a float
    public float getFloat(String key) {
        return (Float)get(key);
    }

    //! Returns the given key as a double
    public double getDouble(String key) {
        return (Double)get(key);
    }

    //! Returns the given key as a String
    public String getString(String key) {
        return (String)get(key);
    }

    //! Returns the given key as an array
    public ArrayList<Object> getList(String key) {
        ArrayList<Object> rv = new ArrayList();
        for (Object elem : (Object[])get(key)) {
            rv.add(elem);
        }
        return rv;
    }

    //! Returns the given key as an absolute date/time value
    public ZonedDateTime getADate(String key) {
        return (ZonedDateTime)get(key);
    }

    //! Returns the given key as a relative date/time value
    public QoreRelativeTime getRDate(String key) {
        return (QoreRelativeTime)get(key);
    }

    //! Returns the given key as an arbitrary-precision numeric value
    public BigDecimal getNumber(String key) {
        return (BigDecimal)get(key);
    }

    //! Returns the given key as a byte array
    public byte[] getBinary(String key) {
        return (byte[])get(key);
    }

    //! Returns the given key as a hash
    public Hash getHash(String key) {
        return (Hash)get(key);
    }

    //! Returns the given key as a callable value
    public QoreClosureMarker getCode(String key) {
        return (QoreClosureMarker)get(key);
    }
}
