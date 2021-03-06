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
    /** @see getAsBool()
     */
    public boolean getBool(String key) {
        return (Boolean)get(key);
    }

    //! Returns the given key as a byte
    /** @see getAsByte()
     */
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
        ArrayList<Object> rv = new ArrayList<Object>();
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

    //! Returns the given key as a boolean
    /** Converts the type of the value if possible before returning
     */
    public boolean getAsBool(String key) {
        Object rv = get(key);
        if (rv instanceof Boolean) {
            return (Boolean)rv;
        }
        if (rv instanceof Number) {
            return ((Number)rv).longValue() != 0 ? true : false;
        }
        String val = String.valueOf(rv);
        if (val.equalsIgnoreCase("true")) {
            return true;
        }
        return Long.parseLong(val) != 0 ? true : false;
    }

    //! Returns the given key as a byte
    /** Converts the type of the value if possible before returning
     */
    public byte getAsByte(String key) {
        Object rv = get(key);
        if (rv instanceof Number) {
            return ((Number)rv).byteValue();
        }
        if (rv instanceof Boolean) {
            return (byte)((Boolean)rv ? 1 : 0);
        }
        // try to convert to a String and then a number
        return Byte.parseByte(String.valueOf(rv));
    }

    //! Returns the given key as a short
    /** Converts the type of the value if possible before returning
     */
    public short getAsShort(String key) {
        Object rv = get(key);
        if (rv instanceof Number) {
            return ((Number)rv).shortValue();
        }
        if (rv instanceof Boolean) {
            return (short)((Boolean)rv ? 1 : 0);
        }
        // try to convert to a String and then a number
        return Short.parseShort(String.valueOf(rv));
    }

    //! Returns the given key as an int
    /** Converts the type of the value if possible before returning
     */
    public int getAsInt(String key) {
        Object rv = get(key);
        if (rv instanceof Number) {
            return ((Number)rv).intValue();
        }
        if (rv instanceof Boolean) {
            return (Boolean)rv ? 1 : 0;
        }
        // try to convert to a String and then a number
        return Integer.parseInt(String.valueOf(rv));
    }

    //! Returns the given key as a long
    /** Converts the type of the value if possible before returning
     */
    public long getAsLong(String key) {
        Object rv = get(key);
        if (rv instanceof Number) {
            return ((Number)rv).longValue();
        }
        if (rv instanceof Boolean) {
            return (Boolean)rv ? 1 : 0;
        }
        // try to convert to a String and then a number
        return Long.parseLong(String.valueOf(rv));
    }

    //! Returns the given key as a float
    /** Converts the type of the value if possible before returning
     */
    public float getAsFloat(String key) {
        Object rv = get(key);
        if (rv instanceof Number) {
            return ((Number)rv).floatValue();
        }
        if (rv instanceof Boolean) {
            return (Boolean)rv ? 1 : 0;
        }
        // try to convert to a String and then a number
        return Float.parseFloat(String.valueOf(rv));
    }

    //! Returns the given key as a double
    /** Converts the type of the value if possible before returning
     */
    public double getAsDouble(String key) {
        Object rv = get(key);
        if (rv instanceof Number) {
            return ((Number)rv).doubleValue();
        }
        if (rv instanceof Boolean) {
            return (Boolean)rv ? 1 : 0;
        }
        // try to convert to a String and then a number
        return Double.parseDouble(String.valueOf(rv));
    }

    //! Returns the given key as a String
    /** Converts the type of the value if possible before returning
     */
    public String getAsString(String key) {
        return String.valueOf(get(key));
    }

    //! Returns a modifiable map of zero entries
    /** @since jni 2.0
     */
    public static Hash of() {
        return new Hash();
    }

    //! Returns a modifiable map of one entry
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0) {
        return new Hash() {
            {
                put(k0, v0);
            }
        };
    }

    //! Returns a modifiable map of two entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
            }
        };
    }

    //! Returns a modifiable map of three entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
            }
        };
    }

    //! Returns a modifiable map of four entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2,
            String k3, Object v3) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
                put(k3, v3);
            }
        };
    }

    //! Returns a modifiable map of five entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2,
            String k3, Object v3,
            String k4, Object v4) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
                put(k3, v3);
                put(k4, v4);
            }
        };
    }

    //! Returns a modifiable map of six entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2,
            String k3, Object v3,
            String k4, Object v4,
            String k5, Object v5) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
                put(k3, v3);
                put(k4, v4);
                put(k5, v5);
            }
        };
    }

    //! Returns a modifiable map of seven entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2,
            String k3, Object v3,
            String k4, Object v4,
            String k5, Object v5,
            String k6, Object v6) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
                put(k3, v3);
                put(k4, v4);
                put(k5, v5);
                put(k6, v6);
            }
        };
    }

    //! Returns a modifiable map of eight entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2,
            String k3, Object v3,
            String k4, Object v4,
            String k5, Object v5,
            String k6, Object v6,
            String k7, Object v7) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
                put(k3, v3);
                put(k4, v4);
                put(k5, v5);
                put(k6, v6);
                put(k7, v7);
            }
        };
    }

    //! Returns a modifiable map of nine entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2,
            String k3, Object v3,
            String k4, Object v4,
            String k5, Object v5,
            String k6, Object v6,
            String k7, Object v7,
            String k8, Object v8) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
                put(k3, v3);
                put(k4, v4);
                put(k5, v5);
                put(k6, v6);
                put(k7, v7);
                put(k8, v8);
            }
        };
    }

    //! Returns a modifiable map of ten entries
    /** @since jni 2.0
     */
    public static Hash of(String k0, Object v0,
            String k1, Object v1,
            String k2, Object v2,
            String k3, Object v3,
            String k4, Object v4,
            String k5, Object v5,
            String k6, Object v6,
            String k7, Object v7,
            String k8, Object v8,
            String k9, Object v9) {
        return new Hash() {
            {
                put(k0, v0);
                put(k1, v1);
                put(k2, v2);
                put(k3, v3);
                put(k4, v4);
                put(k5, v5);
                put(k6, v6);
                put(k7, v7);
                put(k8, v8);
                put(k9, v9);
            }
        };
    }
}
