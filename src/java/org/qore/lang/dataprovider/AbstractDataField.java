/** Java wrapper for the %Qore AbstractDataField class
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

//! Java wrapper for the @ref DataProvider::AbstractDataField class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class AbstractDataField extends QoreObjectWrapper {
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
    public AbstractDataField(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! returns True if this field's type can be assigned from values of the argument type
    public boolean isAssignableFrom(AbstractDataProviderType t) throws Throwable {
        return (boolean)obj.callMethod("isAssignableFrom", t);
    }

    //! returns True if this field's type can be assigned from values of the argument type
    public boolean isAssignableFrom(Type t) throws Throwable {
        return (boolean)obj.callMethod("isAssignableFrom", t);
    }

    //! returns True if this field's type is a list
    public boolean isList() throws Throwable {
        return (boolean)obj.callMethod("isList");
    }

    //! returns True if the field's type must have a value
    public boolean isMandatory() throws Throwable {
        return (boolean)obj.callMethod("isMandatory");
    }

    //! returns the value of the given option on the field's type
    public Object getOptionValue(String opt) throws Throwable {
        return obj.callMethod("getOptionValue", opt);
    }

    //! returns options set on the field's type
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getOptions() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getOptions");
    }

    //! returns supported options on the field's type
    @SuppressWarnings("unchecked")
    public HashMap<String, HashMap<String, Object>> getSupportedOptions() throws Throwable {
        return (HashMap<String, HashMap<String, Object>>)obj.callMethod("getSupportedOptions");
    }

    //! sets the given option on the field's type
    /** @param opt the option to set
        @param value the value to set

        @throw TYPE-OPTION-ERROR invalid option or invalid option type
    */
    public void setOption(String opt, Object value) throws Throwable {
        obj.callMethod("setOption", opt, value);
    }

    //! sets the given options on the field's type
    /** @param options a hash of options, if any options match supported options for this type, they are set

        @throw TYPE-OPTION-ERROR option value has an invalid type
    */
    public void setOptions(Map<String, Object> options) throws Throwable {
        obj.callMethod("setOptions", options);
    }

    //! returns the type name
    public String getTypeName() throws Throwable {
        return (String)obj.callMethod("getTypeName");
    }

    //! sets the default value for the field
    public void setDefaultValue(Object default_value) throws Throwable {
        obj.callMethod("setDefaultValue", default_value);
    }

    //! get default value, if any
    public Object getDefaultValue() throws Throwable {
        return obj.callMethod("getDefaultValue");
    }

    //! returns True if the field's type is not a wildcard type
    public boolean hasType() throws Throwable {
        return (boolean)obj.callMethod("hasType");
    }

    //! returns the value if the value can be assigned to the type
    /** @param value the value to assign to the type

        @return the value to be assigned; can be converted by the type
    */
    public Object acceptsValue(Object value) throws Throwable {
        return obj.callMethod("acceptsValue", value);
    }

    //! returns information about the field as an input field
    /** the \c default_value key is not returned, and the \c type key returns only input information
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getInputInfo() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getInputInfo");
    }

    //! returns information about the field
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getInfo() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getInfo");
    }

    //! returns a field with a "soft" type equivalent to the current type
    /** The base class method returns the same field; this method must be overridden in child classes to return a
        field with a usable "soft" type

        @return a field with a "soft" type equivalent to the current type
    */
    public AbstractDataField getSoftType() throws Throwable {
        return new AbstractDataField((QoreObject)obj.callMethodSave("getSoftType"));
    }

    //! returns the name of the field
    public String getName() throws Throwable {
        return (String)obj.callMethod("getName");
    }

    //! returns the description, if any
    public String getDescription() throws Throwable {
        return (String)obj.callMethod("getDescription");
    }

    //! returns the type of the field
    public AbstractDataProviderType getType() throws Throwable {
        return new AbstractDataProviderType((QoreObject)obj.callMethodSave("getType"));
    }
}
