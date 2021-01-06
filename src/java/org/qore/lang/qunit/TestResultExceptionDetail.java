/** Java wrapper for the %Qore QUnit::TestResulExceptionDetail class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing Exception of a particular type with a particular detail message
/**
    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.QUnit.TestResultExceptionDetail;</tt>
*/
@Deprecated
public class TestResultExceptionDetail extends AbstractTestResult {
    //! creates the object from the exception arguments
    /** @param exceptionType corresponds to the \c "err" key of @ref Qore::ExceptionInfo "ExceptionInfo" (the first
        value of a @ref throw "throw statement")
        @param exceptionDetail corresponds to the \c "desc" key of @ref Qore::ExceptionInfo "ExceptionInfo"
    */
    public TestResultExceptionDetail(String exceptionType, String exceptionDetail) throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultExceptionDetail", exceptionType, exceptionDetail));
    }
}
