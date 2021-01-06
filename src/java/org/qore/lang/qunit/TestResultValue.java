/** Java wrapper for the %Qore QUnit::TestResultValue class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing boolean True
/**
    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.QUnit.TestResultValue;</tt>
*/
@Deprecated
public class TestResultValue extends AbstractTestResult {
    //! creates the object with the given value
    public TestResultValue(Object value) throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultValue", value));
    }
}
