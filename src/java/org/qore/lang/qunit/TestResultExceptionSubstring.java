/** Java wrapper for the %Qore QUnit::TestResultExceptionSubstring class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing Exception of a particular type and matching regexp for detail
public class TestResultExceptionSubstring extends AbstractTestResult {
    //! creates the object from the exception arguments
    /** @param exceptionType corresponds to the \c "err" key of @ref Qore::ExceptionInfo "ExceptionInfo" (the first
        value of a @ref throw "throw statement")
        @param exceptionSubstring a substring to be matched with the \c "desc" key of
        @ref Qore::ExceptionInfo "ExceptionInfo"
    */
    public TestResultExceptionSubstring(String exceptionType, String exceptionSubstring) throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultExceptionSubstring", exceptionType, exceptionSubstring));
    }
}
