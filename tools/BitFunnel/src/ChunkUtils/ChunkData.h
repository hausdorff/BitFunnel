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

#pragma once

#include <memory>                          // std::unqiue_ptr member.
#include <vector>                          // std::vector member.

#include "BitFunnel/NonCopyable.h"         // Inherits from NonCopyable.

namespace BitFunnel
{
    class ChunkStream : public NonCopyable
    {
    public:
        ChunkStream(Term::StreamId id);
        virtual Term::StreamId GetId();

        virtual void AddTerm(char const * term);
        virtual size_t GetTermCount();
    private:
        Term::StreamId m_id;
        size_t m_termCount = 0;
    };

    class ChunkDocument : public NonCopyable
    {
    public:
        ChunkDocument(DocId id);
        virtual DocId GetId();

        virtual void OpenStream(Term::StreamId id);
        virtual void CloseStream();
        virtual size_t GetStreamCount();

        std::shared_ptr<ChunkStream> & operator[] (const int index);

        virtual void AddTermToOpenStream(char const * term);

        virtual void AddSourceText(char const * start, size_t n);
        virtual std::vector<char> & GetSourceText();
    private:
        DocId m_id;
        std::vector<char> m_sourceText;
        std::vector<std::shared_ptr<ChunkStream>> m_streams;
        size_t m_currentStream = 0;
    };
}