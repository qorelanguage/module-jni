/** Java wrapper for the %Qore QUnit::TestResultFailure class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing test call failure, both general (without detail) and specific (with detail)
/**
    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.QUnit.TestResultFailure;</tt>
*/
@Deprecated
public class TestResultFailure extends AbstractTestResult {
    public TestResultFailure() throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultFailure"));
    }

    //! Instantiate an annotated failure with string detail
    public TestResultFailure(String s) throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultFailure", s));
    }
}
