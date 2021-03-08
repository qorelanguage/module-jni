/** Java wrapper for the %Qore AbstractDataProviderType class
 *
 */
package org.qore.lang.dataprovider;

// java imports
import java.util.HashMap;
import java.util.Map;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;
import org.qore.jni.QoreJavaApi;
import org.qore.lang.reflection.Type;

//! Java wrapper for the @ref DataProvider::AbstractDataProviderType class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.DataProvider.AbstractDataProviderType;</tt>
*/
@Deprecated
public class AbstractDataProviderType extends QoreObjectWrapper {
    // static initialization
    static {
        // loads and initializes the Qore library and the jni module (if necessary) and loads the \c DataProvider
        // module
        try {
            QoreJavaApi.initQore();
            QoreJavaApi.callFunction("load_module", "DataProvider");
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates the object as a wrapper for the Qore object
    public AbstractDataProviderType(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! returns supported options
    @SuppressWarnings("unchecked")
    public HashMap<String, HashMap<String, Object>> getSupportedOptions() throws Throwable {
        return (HashMap<String, HashMap<String, Object>>)obj.callMethod("getSupportedOptions");
    }

    //! returns a description of the type as an input type
    /** @return a description of the type as an input type; only the following keys are returned
        - \c name
        - \c types_returned
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getInputInfo() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getInputInfo");
    }

    //! returns a description of the type as a hash
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getInfo() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getInfo");
    }

    //! Returns information on fields supported
    @SuppressWarnings("unchecked")
    public HashMap<String, HashMap<String, Object>> getFieldInfo() throws Throwable {
        return (HashMap<String, HashMap<String, Object>>)obj.callMethod("getFieldInfo");
    }

    //! returns True if this type can be assigned from values of the argument type
    public boolean isAssignableFrom(AbstractDataProviderType t) throws Throwable {
        return (boolean)obj.callMethod("isAssignableFrom", t);
    }

    //! returns True if this type can be assigned from values of the argument type
    public boolean isAssignableFrom(Type t) throws Throwable {
        return (boolean)obj.callMethod("isAssignableFrom", t);
    }

    //! returns True if this type is a list
    public boolean isList() throws Throwable {
        return (boolean)obj.callMethod("isList");
    }

    //! returns True if the type must have a value
    public boolean isMandatory() throws Throwable {
        return (boolean)obj.callMethod("isMandatory");
    }

    //! returns the given field, if present, or @ref nothing if not
    public AbstractDataField getField(String field_name) throws Throwable {
        return new AbstractDataField((QoreObject)obj.callMethod("getField", field_name));
    }

    //! returns True if the type is not a wildcard type
    public boolean hasType() throws Throwable {
        return (boolean)obj.callMethod("hasType");
    }

    //! returns the base type name for the type; must be a standard Qore base type name
    public String getBaseTypeName() throws Throwable {
        return (String)obj.callMethod("getBaseTypeName");
    }

    //! returns the base type code for the type
    public int getBaseTypeCode() throws Throwable {
        return ((Long)obj.callMethod("getBaseTypeCode")).intValue();
    }

    //! returns a hash of native base type code keys where no translations are performed; keys are type codes, not names
    @SuppressWarnings("unchecked")
    public HashMap<String, Boolean> getDirectTypeHash() throws Throwable {
        return (HashMap<String, Boolean>)obj.callMethod("getDirectTypeHash");
    }

    //! returns True if the type also accepts @ref nothing
    public boolean isOrNothingType() throws Throwable {
        return (boolean)obj.callMethod("isOrNothingType");
    }

    //! get the given field type if it exists, otherwise return @ref nothing
    public AbstractDataProviderType getFieldType(String field_name) throws Throwable {
        return new AbstractDataProviderType((QoreObject)obj.callMethod("getFieldType", field_name));
    }

    //! returns the value of the given option
    public Object getOptionValue(String opt) throws Throwable {
        return obj.callMethod("getOptionValue", opt);
    }

    //! returns options set on the type
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getOptions() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getOptions");
    }

    //! sets the given option on the type
    /** @param opt the option to set
        @param value the value to set

        @throw TYPE-OPTION-ERROR invalid option or invalid option type
    */
    public void setOption(String opt, Object value) throws Throwable {
        obj.callMethod("setOption", opt, value);
    }

    //! sets options on the type
    /** @param options the options to set; unsupported options are ignored

        @throw TYPE-OPTION-ERROR invalid option type
    */
    public void setOptions(Map<String, Object> options) throws Throwable {
        obj.callMethod("setOptions", options);
    }

    //! returns a "soft" type equivalent to the current type
    /** The base class method returns the same type; this method must be overridden in child classes to return a
        usable "soft" type

        @return a "soft" type equivalent to the current type
    */
    public AbstractDataProviderType getSoftType() throws Throwable {
        return new AbstractDataProviderType((QoreObject)obj.callMethod("getSoftType"));
    }

    //! returns the type name
    public String getName() throws Throwable {
        return (String)obj.callMethod("getName");
    }

    //! returns the base type for the type, if any
    public Type getValueType() throws Throwable {
        QoreObject rv = (QoreObject)obj.callMethod("getValueType");
        if (rv != null) {
            return new Type(rv);
        }
        return null;
    }

    //! returns the subtype (for lists or hashes) if there is only one
    public AbstractDataProviderType getElementType() throws Throwable {
        QoreObject rv = (QoreObject)obj.callMethod("getElementType");
        if (rv != null) {
            return new AbstractDataProviderType(rv);
        }
        return null;
    }

    //! returns the fields of the data structure; if any
    @SuppressWarnings("unchecked")
    public HashMap<String, AbstractDataField> getFields() throws Throwable {
        return (HashMap<String, AbstractDataField>)obj.callMethod("getFields");
    }

    //! returns a hash of types accepted by this type; keys are type names
    @SuppressWarnings("unchecked")
    public HashMap<String, Boolean> getAcceptTypeHash() throws Throwable {
        return (HashMap<String, Boolean>)obj.callMethod("getAcceptTypeHash");
    }

    //! returns a hash of types returned by this type; keys are type names
    @SuppressWarnings("unchecked")
    public HashMap<String, Boolean> getReturnTypeHash() throws Throwable {
        return (HashMap<String, Boolean>)obj.callMethod("getReturnTypeHash");
    }

    //! returns the value if the value can be assigned to the type
    /** @param value the value to assign to the type

        @return the value to be assigned; can be converted by the type
    */
    public Object acceptsValue(Object value) throws Throwable {
        return obj.callMethod("acceptsValue", value);
    }

    //! returns an appropriate object for the given type
    public static AbstractDataProviderType get(Type type, Map<String, Object> options) throws Throwable {
        return new AbstractDataProviderType((QoreObject)QoreJavaApi.callStaticMethodSave("AbstractDataProviderType",
            "get", type, options));
    }

    //! returns an appropriate object for the given type
    public static AbstractDataProviderType get(Type type) throws Throwable {
        return new AbstractDataProviderType((QoreObject)QoreJavaApi.callStaticMethodSave("AbstractDataProviderType",
            "get", type));
    }

    //! returns an appropriate object for the given type
    public static AbstractDataProviderType get(String typename, Map<String, Object> options) throws Throwable {
        return new AbstractDataProviderType((QoreObject)QoreJavaApi.callStaticMethodSave("AbstractDataProviderType",
            "get", typename, options));
    }

    //! returns an appropriate object for the given type
    public static AbstractDataProviderType get(String typename) throws Throwable {
        return new AbstractDataProviderType((QoreObject)QoreJavaApi.callStaticMethodSave("AbstractDataProviderType",
            "get", typename));
    }
}
