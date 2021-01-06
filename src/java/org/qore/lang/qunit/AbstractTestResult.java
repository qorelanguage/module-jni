/** Java wrapper for the %Qore QUnit::AbstractTestResult class
 *
 */
package org.qore.lang.qunit;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;

//! The base class for test results
/**
    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.QUnit.AbstractTestResult;</tt>
*/
@Deprecated
public class AbstractTestResult extends QoreObjectWrapper {
    //! creates the object based on the given Qore object
    protected AbstractTestResult(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! compare two results for equality
    public boolean equals(AbstractTestResult r) throws Throwable {
        return (boolean)obj.callMethod("equals", r);
    }

    //! returns a string reprensentation of the result
    public String toString() {
        try {
            return (String)obj.callMethod("toString");
        } catch (Throwable e) {
            return e.toString();
        }
    }
}
