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

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IChunkManifestIngestor.h"
#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "CmdLineParser/CmdLineParser.h"
#include "ChunkUtils.h"
#include "ChunkUtils/ChunkProcessor.h"


namespace BitFunnel
{
    ChunkUtils::ChunkUtils(IFileSystem& fileSystem)
      : m_fileSystem(fileSystem)
    {
    }


    int ChunkUtils::Main(std::istream& /*input*/,
                         std::ostream& output,
                         int argc,
                         char const *argv[])
    {
        // TODO: Clean up all the unused variables in here.
        CmdLine::CmdLineParser parser(
            "ChunkUtils",
            "Ingest documents and compute statistics about them.");

        CmdLine::RequiredParameter<char const *> manifestFileName(
            "manifestFile",
            "Path to a file containing the paths to the chunk files to be ingested. "
            "One chunk file per line. Paths are relative to working directory.");

        CmdLine::RequiredParameter<char const *> outputFile(
            "outFile",
            "The output file to generate. ");

        parser.AddParameter(manifestFileName);
        parser.AddParameter(outputFile);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                LoadAndProcessChunkFileList(output,
                                            outputFile,
                                            manifestFileName);
                returnCode = 0;
            }
            catch (RecoverableError e)
            {
                output << "Error: " << e.what() << std::endl;
            }
            catch (...)
            {
                output << "Unexpected error.";
            }
        }

        return returnCode;
    }


    std::shared_ptr<ChunkProcessor>
        ChunkUtils::LoadChunkFile(std::ostream& /*output*/,
                                 std::vector<std::string> filePaths,
                                 size_t index) const
    {
        if (index >= filePaths.size())
        {
            FatalError error("ChunkManifestIngestor: chunk index out of range.");
            throw error;
        }

        auto input = m_fileSystem.OpenForRead(filePaths[index].c_str(),
                                              std::ios::binary);

        if (input->fail())
        {
            std::stringstream message;
            message << "Failed to open chunk file '"
                << filePaths[index]
                << "'";
            throw FatalError(message.str());
        }

        input->seekg(0, input->end);
        auto length = input->tellg();
        input->seekg(0, input->beg);

        std::vector<char> chunkData;
        chunkData.reserve(static_cast<size_t>(length) + 1ull);
        chunkData.insert(chunkData.begin(),
                         (std::istreambuf_iterator<char>(*input)),
                         std::istreambuf_iterator<char>());

        // NOTE: The act of constructing a ChunkProcessor causes the bytes in
        // chunkData to be parsed and processed.
        return std::unique_ptr<ChunkProcessor>(
            new ChunkProcessor(
                &chunkData[0],
                &chunkData[0] + chunkData.size()));
    }


    void ChunkUtils::WriteChunkFileStatisticsHeader(
        std::ostream& statsOutput) const
    {
        statsOutput << "docId,lengthOfAllStreams" << std::endl;
    }


    void ChunkUtils::WriteChunkFileStatistics(
        std::ostream& statsOutput,
        std::shared_ptr<ChunkProcessor> chunks) const
    {
        for (size_t i = 0; i < chunks->GetDocumentCount(); ++i) {
            auto chunk = (*chunks)[i];

            // Print statistics.
            size_t lengthOfAllStreams = 0;
            for (size_t j = 0; j < (*chunk).GetStreamCount(); ++j) {
                lengthOfAllStreams += (*chunk)[j]->GetTermCount();
            }

            statsOutput
                << chunk->GetId() << "," << lengthOfAllStreams << std::endl;
        }
    }


    void ChunkUtils::WriteChunkFiles(
        std::ostream& chunkfileOutput,
        std::shared_ptr<ChunkProcessor> chunks) const
    {
        for (size_t i = 0; i < chunks->GetDocumentCount(); ++i) {
            auto chunk = (*chunks)[i];

            // TODO: Allow ability to filter out chunks we don't want.
            // Write out chunk file.
            auto sourceText = chunk->GetSourceText();
            for (size_t j = 0; j < sourceText.size(); ++j) {
                chunkfileOutput << sourceText[j];
            }
        }
    }


    void ChunkUtils::LoadAndProcessChunkFileList(
        std::ostream& output,
        char const * chunkFilePath,
        char const * chunkListFileName) const
    {
        // TODO: Add try/catch around file operations.
        output
            << "Loading chunk list file '" << chunkListFileName << "'" << std::endl
            << "Output file: '" << chunkFilePath << "'"<< std::endl;

        std::vector<std::string> filePaths = ReadLines(
            m_fileSystem,
            chunkListFileName);

        output << "Reading " << filePaths.size() << " files\n";

        bool writeChunks = false;
        std::unique_ptr<std::ostream> outputFileStream =
            m_fileSystem.OpenForWrite(
                chunkFilePath,
                std::ios::binary);
        if (!writeChunks)
        {
            WriteChunkFileStatisticsHeader(*outputFileStream);
        }

        size_t totalDocumentCount = 0;
        Stopwatch stopwatch;

        for (size_t i = 0; i < filePaths.size(); ++i) {
            // Process a single chunk file.
            auto chunks = LoadChunkFile(output, filePaths, i);

            size_t documentCount = chunks->GetDocumentCount();

            if (!writeChunks)
            {
                WriteChunkFileStatistics(*outputFileStream, chunks);
            }
            else
            {
                WriteChunkFiles(*outputFileStream, chunks);
            }

            totalDocumentCount += documentCount;
        }

        const double elapsedTime = stopwatch.ElapsedTime();

        output
            << "Ingestion complete." << std::endl
            << "  Ingestion time: " << elapsedTime << std::endl
            << "  Total document count: " << totalDocumentCount << std::endl;
    }
}
