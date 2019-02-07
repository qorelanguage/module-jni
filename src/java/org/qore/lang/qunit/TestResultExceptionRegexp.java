/** Java wrapper for the %Qore QUnit::TestResultExceptionRegexp class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing Exception of a particular type and matching regexp for detail
public class TestResultExceptionRegexp extends AbstractTestResult {
    //! creates the object from the exception arguments
    /** @param exceptionType corresponds to the \c "err" key of @ref Qore::ExceptionInfo "ExceptionInfo" (the first
        value of a @ref throw "throw statement")
        @param exceptionRegexp a regular expression string to be matched with the \c "desc" key of
        @ref Qore::ExceptionInfo "ExceptionInfo"
    */
    public TestResultExceptionRegexp(String exceptionType, String exceptionRegexp) throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultExceptionRegexp", exceptionType, exceptionRegexp));
    }
}
