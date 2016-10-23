#pragma once

#include "BitFunnel/BitFunnelTypes.h"           // DocId.
#include "BitFunnel/IInterface.h"               // Base class.
#include "BitFunnel/Term.h"                     // Term::StreamId parameter.

namespace BitFunnel
{
    class IEvents : public IInterface
    {
    public:
        virtual void OnFileEnter() = 0;
        virtual void OnDocumentEnter(DocId id) = 0;
        virtual void OnStreamEnter(Term::StreamId id) = 0;
        virtual void OnTerm(char const * term) = 0;
        virtual void OnStreamExit() = 0;
        virtual void OnDocumentExit(size_t bytesRead) = 0;
        virtual void OnFileExit() = 0;
    };

    class IChunkReader : public IInterface
    {
    // DESIGN NOTE: Would like to use const char * to avoid string copy and
    // memory allocation during ingestion. This may require reading the entire
    // file into a buffer before parsing.
    // DESIGN NOTE: Need to add arena allocators.
    };
}