// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <iostream>

#include "BitFunnel/Index/Factories.h"
#include "ChunkData.h"

namespace BitFunnel
{
    //
    // ChunkStream
    //
    ChunkStream::ChunkStream(Term::StreamId id)
      : m_id(id)
    {
    }

    Term::StreamId ChunkStream::GetId()
    {
        return m_id;
    }

    void ChunkStream::AddTerm(char const * /*term*/)
    {
        // std::cout << term << std::endl;
        // TODO: Add term to the ChunkStream.
        ++m_termCount;
    }

    size_t ChunkStream::GetTermCount()
    {
        return m_termCount;
    }


    //
    // ChunkDocument
    //
    ChunkDocument::ChunkDocument(DocId id)
      : m_id(id)
    {
    }

    DocId ChunkDocument::GetId()
    {
        return m_id;
    }

    void ChunkDocument::OpenStream(Term::StreamId id)
    {
        m_streams.push_back(std::unique_ptr<ChunkStream>(new ChunkStream(id)));
    }

    void ChunkDocument::CloseStream()
    {
        ++m_currentStream;
    }

    void ChunkDocument::AddTermToOpenStream(char const * term)
    {
        m_streams[m_currentStream]->AddTerm(term);
    }

    size_t ChunkDocument::GetStreamCount()
    {
        return m_streams.size();
    }

    void ChunkDocument::AddSourceText(char const * start, size_t n)
    {
        m_sourceText = std::vector<char>(start, start + n);
    }

    std::vector<char> & ChunkDocument::GetSourceText()
    {
        return m_sourceText;
    }
}