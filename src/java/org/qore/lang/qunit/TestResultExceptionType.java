/** Java wrapper for the %Qore QUnit::TestResulExceptionType class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing Exception of a particular type
/**
    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.QUnit.TestResultExceptionType;</tt>
*/
@Deprecated
public class TestResultExceptionType extends AbstractTestResult {
    //! creates the object from the exception arguments
    /** @param exceptionType corresponds to the \c "err" key of @ref Qore::ExceptionInfo "ExceptionInfo" (the first
        value of a @ref throw "throw statement")
    */
    public TestResultExceptionType(String exceptionType) throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultExceptionType", exceptionType));
    }
}
