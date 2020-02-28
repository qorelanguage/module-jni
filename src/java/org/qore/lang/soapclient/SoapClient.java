/** Java wrapper for the %Qore SoapClient class
 *
 */
package org.qore.lang.soapclient;

// java imports
import java.util.Map;
import java.util.HashMap;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;
import org.qore.jni.QoreJavaApi;
import org.qore.lang.*;

//! Java wrapper for the @ref SoapClient::SoapClient class in %Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 */
public class SoapClient extends HTTPClient {
    // static initialization
    static {
        // load the SoapClient module
        try {
            QoreJavaApi.initQore();
            QoreJavaApi.callFunction("load_module", "SoapClient");
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates the object
    public SoapClient(QoreObject ds) {
        super(ds);
    }

    //! creates the object based on a %WSDL which is parsed to a @ref WSDL::WebService "WebService" object which provides the basis for all communication with this object
    /** one of either the \c wsdl or \c wsdl_file keys is required in the hash given to the constructor or an exception will be thrown
        @param h valid option keys:
        - \c wsdl: the URL of the web service or a @ref WSDL::WebService "WebService" object itself
        - \c wsdl_file: a path to use to load the %WSDL and create the @ref WSDL::WebService "WebService" object
        - \c url: override the target URL given in the %WSDL
        - \c send_encoding: a @ref EncodingSupport "send data encoding option" or the value \c "auto" which means to use automatic encoding; if not present defaults to no content-encoding on sent message bodies
        - \c content_encoding: for possible values, see @ref EncodingSupport; this sets the send encoding (if the \c "send_encoding" option is not set) and the requested response encoding
        - [\c service]: in case multiple service entries are found in the WSDL, give the one to be used here
        - [\c port]: in case multiple port entries are found in the WSDL, give the one to be used here; note that the SOAP binding is resolved from the \a service and \a port options.
        - [\c log]: a log closure or call reference taking a single string giving the log message
        - [\c dbglog]: a log closure or call reference taking a single string giving the debug log message
        - also all options from @ref SoapClient::SoapClient::HTTPOptions "SoapClient::HTTPOptions", which are passed to @ref Qore::HTTPClient::constructor() "HTTPClient::constructor()"
    */
    public SoapClient(Map<String, Object> h) throws Throwable {
        super(h);
    }

    //! makes a server call with the given operation, arguments, options, and optional info hash reference and returns the result
    /** @param operation the SOAP operation to use to serialize the request; if the operation is not known to the underlying @ref WSDL::WebService "WebService" class, an exception will be thrown
        @param args the arguments to the SOAP operation
        @param opts an optional hash of options for the call as follows:
        - \c soap_header: a hash giving SOAP header information, if required by the message
        - \c http_header: a hash giving HTTP header information to include in the message (does not override automatically-generated SOAP message headers)
        - \c xml_opts: an integer XML generation option code; see @ref xml_generation_constants for possible values; combine multiple codes with binary or (\c |)
        - \c soapaction: an optional string that will override the SOAPAction for the request; en empty string here will prevent the SOAPAction from being sent
        - \c binding: the SOAP binding name, if not provided the first binding assigned to the operation will be used

        @return a hash with the following keys:
        - \c hdr: a hash of message headers
        - \c body: the serialized message body

        @throw WSDL-OPERATION-ERROR the operation is not defined in the WSDL
        @throw WSDL-BINDING-ERROR the binding is not assigned to a SOAP operation in the WSDL
        @throw HTTP-CLIENT-RECEIVE-ERROR this exception is thrown when the SOAP server returns an HTTP error code; if a SOAP fault is returned, then it is deserialized and returned in the \a arg key of the exception hash

        @note this method can throw any exception that @ref Qore::HTTPClient::send() "HTTPClient::send()" can throw as well as any XML parsing errors thrown by @ref Qore::XML::parse_xml() "parse_xml()"
    */
    public Object callOperation(String operation, Object args, Map<String, Object> opts) throws Throwable {
        return obj.callMethod("callOperation", operation, args, opts);
    }

    //! returns a hash of information about the current WSDL
    /** @return a hash with the following keys:
        - \c "service": the name of the SOAP service used
        - \c "pore": the name of the SOAP port used
        - \c "binding": the name of the binding used
        - \c "url": the target URL
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getInfo() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getInfo");
    }

    //! change the data content encoding (compression) option for the object; see @ref EncodingSupport for valid options
    /** @par Example:
        @code{.java}
sc.setSendEncoding("gzip");
        @endcode

        The default is to send requests unencoded/uncompressed.

        @param enc the data content encoding (compression) option for the object; see @ref EncodingSupport for valid options; if the value \c "auto" is passed then \c "gzip" encoding is used

        @throw SOAPCLIENT-ERROR invalid or unsupported data content encoding / compression option

        @see
        - @ref setContentEncoding()
        - @ref getSendEncoding()
    */
    public void setSendEncoding(String enc) throws Throwable {
        obj.callMethod("setSendEncoding", enc);
    }

    //! sets the request and desired response encoding for the object; see @ref EncodingSupport for valid options
    /** @par Example:
        @code{.java}
soap.setContentEncoding("gzip");
        @endcode

        @param enc the data content encoding (compression) option for requests and the desired response content encoding for the object; see @ref EncodingSupport for valid options; if the value \c "auto" is passed then \c "gzip" encoding is used for outgoing requests and requested for responses

        @throw SOAPCLIENT-ERROR invalid or unsupported data content encoding / compression option

        @see
        - @ref getSendEncoding()
        - @ref setSendEncoding()

        @since %SoapClient 0.2.4
    */
    public void setContentEncoding(String enc) throws Throwable {
        obj.callMethod("setContentEncoding", enc);
    }

    //! adds default headers to each request; these headers will be sent in all requests but can be overridden in requests as well
    /** @par Example:
        @code{.java}
// disable gzip and bzip encoding in responses
soap.addDefaultHeaders(hdrs);
        @endcode

        @param h a hash of headers to add to the default headers to send on each request

        @note default headers can also be set in the constructor

        @see @ref getDefaultHeaders()
    */
    public void addDefaultHeaders(Map<String, Object> h) throws Throwable {
        obj.callMethod("addDefaultHeaders", h);
    }

    //! returns the hash of default headers to sent in all requests
    /** @par Example:
        @code{.java}
HashMap<String, Object> h = soap.getDefaultHeaders();
        @endcode

        @return the hash of default headers to sent in all requests

        @note default headers can be set in the constructor and in addDefaultHeaders()

        @see @ref setDefaultHeaders()
    */
    @SuppressWarnings("unchecked")
    public HashMap<String, Object> getDefaultHeaders() throws Throwable {
        return (HashMap<String, Object>)obj.callMethod("getDefaultHeaders");
    }

    //! returns the current data content encoding (compression) object or @ref nothing if no encoding option is set; see @ref EncodingSupport for valid options
    /** @par Example:
        @code{.java}
String ce = soap.getSendEncoding();
        @endcode

        @return the current data content encoding (compression) object or @ref nothing if no encoding option is set; see @ref EncodingSupport for valid options

        @see
        - @ref setContentEncoding()
        - @ref setSendEncoding()
    */
    public String getSendEncoding() throws Throwable {
        return (String)obj.callMethod("getSendEncoding");
    }

    //! sends a log message to the log closure or call reference, if any
    public void log(String msg) throws Throwable {
        obj.callMethod("log", msg);
    }

    //! sends a log message to the debug log closure or call reference, if any
    public void dbglog(String msg) throws Throwable {
        obj.callMethod("dbglog", msg);
    }
}
