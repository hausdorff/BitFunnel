#include <stdexcept>

#include "DocumentHandleInternal.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // DocumentHandle
    //
    //*************************************************************************
    void* DocumentHandle::AllocateVariableSizeBlob(VariableSizeBlobId /*id*/, size_t /*byteSize*/)
    {
        throw std::runtime_error("Not implemented.");
    }


    void* DocumentHandle::GetVariableSizeBlob(VariableSizeBlobId /*id*/) const
    {
        throw std::runtime_error("Not implemented.");
    }


    void* DocumentHandle::GetFixedSizeBlob(FixedSizeBlobId /*id*/) const
    {
        throw std::runtime_error("Not implemented.");
    }


    void DocumentHandle::AssertFact(FactHandle /*fact*/, bool /*value*/)
    {
        throw std::runtime_error("Not implemented.");
    }


    void DocumentHandle::AddPosting(Term const & /*term*/)
    {
        throw std::runtime_error("Not implemented.");
    }


    void DocumentHandle::Expire()
    {
        throw std::runtime_error("Not implemented.");
    }


    DocId DocumentHandle::GetDocId() const
    {
        throw std::runtime_error("Not implemented.");
    }


    DocumentHandle::DocumentHandle(Slice* /*slice*/, DocIndex /*index*/)
    {
    }


    //*************************************************************************
    //
    // DocumentHandleInternal
    //
    //*************************************************************************

    DocumentHandleInternal::DocumentHandleInternal()
        : DocumentHandle(nullptr, c_invalidDocIndex)
    {
    }


    DocumentHandleInternal::DocumentHandleInternal(Slice* slice, DocIndex index)
        : DocumentHandle(slice, index)
    {
    }


    DocumentHandleInternal::DocumentHandleInternal(DocumentHandle const & handle)
        : DocumentHandle(handle)
    {
    }


    Slice* DocumentHandleInternal::GetSlice() const
    {
        return m_slice;
    }


    DocIndex DocumentHandleInternal::GetIndex() const
    {
        return m_index;
    }


    void DocumentHandleInternal::Activate()
    {
        throw std::runtime_error("Not implemented.");
    }
}
