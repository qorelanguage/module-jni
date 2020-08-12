/** Java wrapper for the %Qore AbstractDataProcessor class
 *
 */
package org.qore.lang.dataprovider;

// jni module imports
import org.qore.lang.dataprovider.AbstractDataProviderType;

//! Java AbstractDataProcessor class
/**
    @since jni 1.1.3
*/
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
    /** @param _data the data to process

        @throw DPE-SKIP-DATA throw this exception to tell the
        @ref DataProvider::DataProviderPipeline "DataProviderPipeline" skip processing the data for the rest of the
        queue

        @note
        - Calls @ref submitImpl() on the data to do the actual processing
        - Accept and return type information is not enforced in this method; it must be enforced in submitImpl()
    */
    public Object submit(Object _data) throws Throwable {
        return submitImpl(_data);
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
    /** @param _data the data to process

        @throw DPE-SKIP-DATA throw this exception to tell the
        @ref DataProvider::DataProviderPipeline "DataProviderPipeline" to skip processing the data for the rest of the
        queue
    */
    protected abstract Object submitImpl(Object _data) throws Throwable;

    //! Returns true if the data processor supports bulk operation
    /** @return true if the data processor supports bulk operation
    */
    protected abstract boolean supportsBulkApiImpl();
}

