/** Java wrapper for the %Qore AbstractIterator class
 *
 */
package org.qore.lang;

// jni module imports
import org.qore.jni.QoreObject;
import org.qore.jni.QoreObjectWrapper;
import org.qore.jni.QoreJavaApi;

//! Java wrapper for the @ref Qore::AbstractIterator class in Qore
/** @note Loads and initializes the Qore library and the jni module in static initialization if necessary
 *  @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
 *  <tt>import qore.Qore.AbstractIterator;</tt>
 */
@Deprecated
public class AbstractIterator extends QoreObjectWrapper {
    // static initialization
    static {
        // loads and initializes the Qore library and the jni module (if necessary)
        try {
            QoreJavaApi.initQore();
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    //! creates the object as a wrapper for the Qore object
    public AbstractIterator(QoreObject obj) throws Throwable {
        super(obj);
    }

    //! Moves the current position to the next element; returns @ref False if there are no more elements
    /** This method will return @ref True again after it returns @ref False once if the object being iterated is not empty, otherwise it will always return @ref False.
        The iterator object should not be used after this method returns @ref False

        @return @ref False if there are no more elements (in which case the iterator object is invalid and should not be used); @ref True if successful (meaning that the iterator object is valid)
    */
    public boolean next() throws Throwable {
        return (boolean)obj.callMethod("next");
    }

    //! returns the current value
    /** @return the current value

        @throw INVALID-ITERATOR the iterator is not pointing at a valid element

        @see AbstractIterator::valid()
    */
    public Object getValue() throws Throwable {
        return obj.callMethod("getValue");
    }

    //! returns true if the iterator is currently pointing at a valid element, false if not
    /** @return true if the iterator is currently pointing at a valid element, false if not
    */
    public boolean valid() throws Throwable {
        return (boolean)obj.callMethod("valid");
    }
}
