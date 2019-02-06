/** Java wrapper for the %Qore QUnit::TestResultSuccess class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreJavaApi;

//! Class representing boolean True
public class TestResultSuccess extends AbstractTestResult {
    public TestResultSuccess() throws Throwable {
        super(QoreJavaApi.newObjectSave("QUnit::TestResultSuccess"));
    }
}
