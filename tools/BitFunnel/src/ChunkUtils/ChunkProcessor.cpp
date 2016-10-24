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

#include "BitFunnel/Index/Factories.h"
#include "ChunkProcessor.h"


namespace BitFunnel
{
    ChunkProcessor::ChunkProcessor(
        char const * start,
        char const * end)
    {
        // TODO: Possibly change this to `OnFileExit` move the current chunk
        // out to some other thing that holds chunks.
        // TODO: Add flag for keeping text data, or not.
        Factories::CreateChunkReader(start, end, *this);
    }


    size_t ChunkProcessor::GetDocumentCount()
    {
        return m_chunks.size();
    }


    std::shared_ptr<ChunkDocument> & ChunkProcessor::operator[] (const int index)
    {
        return m_chunks[index];
    }


    void ChunkProcessor::OnFileEnter()
    {
    }


    void ChunkProcessor::OnDocumentEnter(DocId id, char const * start)
    {
        m_currentChunkStart = start;
        m_chunks.push_back(
            std::shared_ptr<ChunkDocument>(new ChunkDocument(id)));
    }


    void ChunkProcessor::OnStreamEnter(Term::StreamId id)
    {
        m_chunks[m_currentChunkIndex]->OpenStream(id);
    }


    void ChunkProcessor::OnTerm(char const * term)
    {
        m_chunks[m_currentChunkIndex]->AddTermToOpenStream(term);
    }


    void ChunkProcessor::OnStreamExit()
    {
        m_chunks[m_currentChunkIndex]->CloseStream();
    }


    void ChunkProcessor::OnDocumentExit(size_t bytesRead)
    {
        m_chunks[m_currentChunkIndex]->AddSourceText(m_currentChunkStart,
                                                     bytesRead);
        ++m_currentChunkIndex;
        // TODO: when we enter a new document, we will blow away the old
        // unique_ptr to the `ChunkDocument`. We should persist it here,
        // probably.

        // m_currentChunk->CloseDocument(bytesRead);
        // m_ingestor.Add(m_currentChunk->GetDocId(), *m_currentChunk);
        // if (m_cacheDocuments)
        // {
        //     DocId id = m_currentChunk->GetDocId();
        //     m_ingestor.GetDocumentCache().Add(std::move(m_currentChunk),
        //                                       id);
        // }
        // else
        // {
        //     m_currentChunk.reset(nullptr);
        // }
    }


    void ChunkProcessor::OnFileExit()
    {
    }
}
