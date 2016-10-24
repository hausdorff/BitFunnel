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

#include <memory>
#include <vector>

#include "ChunkUtils/ChunkProcessor.h"
#include "IExecutable.h"


namespace BitFunnel
{
    class IFileSystem;

    class ChunkUtils : public IExecutable
    {
    public:
        ChunkUtils(IFileSystem& fileSystem);

        //
        // IExecutable methods
        //
        virtual int Main(std::istream& input,
                         std::ostream& output,
                         int argc,
                         char const *argv[]) override;

    private:
        std::shared_ptr<ChunkProcessor> LoadChunkFile(
            std::ostream& output,
            std::vector<std::string> filePaths,
            size_t index) const;

        void WriteChunkFileStatisticsHeader(std::ostream& statsOutput) const;

        void WriteChunkFileStatistics(
            std::ostream& statsOutput,
            std::shared_ptr<ChunkProcessor> chunks) const;

        void WriteChunkFiles(
            std::ostream& chunkfileOutput,
            std::shared_ptr<ChunkProcessor> chunks) const;

        void LoadAndProcessChunkFileList(
            std::ostream& output,
            char const * intermediateDirectory,
            char const * chunkListFileName) const;

        IFileSystem& m_fileSystem;
    };
}
