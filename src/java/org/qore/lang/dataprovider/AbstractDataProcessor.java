/** Java wrapper for the %Qore AbstractDataProcessor class
 *
 */
package org.qore.lang.dataprovider;

// jni module imports
import org.qore.lang.dataprovider.AbstractDataProviderType;
import org.qore.jni.QoreException;
import org.qore.jni.QoreClosure;

//! Java AbstractDataProcessor class
/**
    @since jni 1.2

    @deprecated Use @ref jni_dynamic_import_qore_in_java "dynamic imports" instead:
    <tt>import qoremod.DataProvider.AbstractDataProcessor;</tt>
*/
@Deprecated
public abstract class AbstractDataProcessor {
    //! Returns the expected type of data to be submitted, if available
    /** @return the expected type of data to be submitted, if available

        @note Calls getExpectedTypeImpl() to provide the return value
    */
    public AbstractDataProviderType getExpectedType() {
        return getExpectedTypeImpl();
    }

    //! Returns the type of data that will be returned, if available
    /** @return the type of data that will be returned, if available

        @note Calls getReturnTypeImpl() to provide the return value
    */
    public AbstractDataProviderType getReturnType() {
        return getReturnTypeImpl();
    }

    //! Returns true if the data processor supports bulk operation
    /** @return true if the data processor supports bulk operation

        @note Calls @ref supportsBulkApiImpl() to return the answer
    */
    public boolean supportsBulkApi() {
        return supportsBulkApiImpl();
    }

    //! Submits the data for processing
    /** @par Example
        @code{.java}
void submitImpl(QoreClosure enqueue, Object _data) throws Throwable {
    // .. process the data and submit the modified data for onward processing
    enqueue.call(_data);
}
        enqueue.call(new_rec);
        @endcode

        @param enqueue s closure taking a single arugment that enqueues the processed data for the next step in the
        pipeline; if no data should be processed onwards, do not call enqueue; if only one record should be processed
        onwards, then \a enqueue should be called only once; if multiple records are generated from the input data,
        then call it once for each generated record; prototype: @code{.py} code enqueue = sub (auto qdata) {} @endcode
        @param _data the data to process

        @note
        - Calls @ref submitImpl() on the data to do the actual processing
        - Accept and return type information is not enforced in this method; it must be enforced in submitImpl()
        - Pipeline data can be of any type
    */
    public void submit(QoreClosure enqueue, Object _data) throws Throwable {
        submitImpl(enqueue, _data);
    }

    //! Returns the expected type of data to be submitted, if available
    /** This base class method returns @ref nothing; reimplement in subclasses to provide a type
    */
    protected AbstractDataProviderType getExpectedTypeImpl() {
        return null;
    }

    //! Returns the type of data that will be returned, if available
    /** This base class method returns @ref nothing; reimplement in subclasses to provide a type
    */
    protected AbstractDataProviderType getReturnTypeImpl() {
        return null;
    }

    //! Submits the data for processing
    /** @par Example
        @code{.java}
void submitImpl(QoreClosure enqueue, Object _data) throws Throwable {
    // .. process the data and submit the modified data for onward processing
    enqueue.call(_data);
}
        enqueue.call(new_rec);
        @endcode

        @param enqueue s closure taking a single arugment that enqueues the processed data for the next step in the
        pipeline; if no data should be processed onwards, do not call enqueue; if only one record should be processed
        onwards, then \a enqueue should be called only once; if multiple records are generated from the input data,
        then call it once for each generated record; prototype: @code{.py} code enqueue = sub (auto qdata) {} @endcode
        @param _data the data to process

        @note Pipeline data can be of any type
    */
    protected abstract void submitImpl(QoreClosure enqueue, Object _data) throws Throwable;

    //! Returns true if the data processor supports bulk operation
    /** @return true if the data processor supports bulk operation
    */
    protected abstract boolean supportsBulkApiImpl();
}

