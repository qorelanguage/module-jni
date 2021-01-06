/** Java wrapper for the %Qore QUnit::TestResultSuccess class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing boolean True
/**
    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.QUnit.TestResultSuccess;</tt>
*/
@Deprecated
public class TestResultSuccess extends AbstractTestResult {
    public TestResultSuccess() throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultSuccess"));
    }
}
