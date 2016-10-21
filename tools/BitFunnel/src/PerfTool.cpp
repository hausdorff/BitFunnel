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

#include <chrono>
#include <iostream>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IChunkManifestIngestor.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/QueryPipeline.h"
#include "BitFunnel/Plan/IMatchVerifier.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Plan/TermMatchTreeEvaluator.h"
#include "BitFunnel/Utilities/Factories.h"
#include "CmdLineParser/CmdLineParser.h"
#include "Environment.h"
#include "PerfTool.h"
#include "TaskFactory.h"
#include "TaskPool.h"

#include "Manifest.h"

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;


namespace BitFunnel
{
    PerfTool::PerfTool(IFileSystem& fileSystem)
      : m_fileSystem(fileSystem)
    {
    }


    int PerfTool::Main(std::istream& input,
                   std::ostream& output,
                   int argc,
                   char const *argv[])
    {
        CmdLine::CmdLineParser parser(
            "StatisticsBuilder",
            "Ingest documents and compute statistics about them.");

        CmdLine::RequiredParameter<char const *> path(
            "path",
            "Path to a tmp directory. "
            "Something like /tmp/ or c:\\temp\\, depending on platform..");

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> gramSize(
            "gramsize",
            "Set the maximum ngram size for phrases.",
            1u);

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> threadCount(
            "threads",
            "Set the thread count for ingestion and query processing.",
            1u);

        parser.AddParameter(path);
        parser.AddParameter(gramSize);
        parser.AddParameter(threadCount);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                // TODO: these casts can be removed when gramSize and
                // threadCount are fixed to be unsigned.
                Go(input,
                   output,
                   path,
                   static_cast<size_t>(gramSize),
                   static_cast<size_t>(threadCount));
                returnCode = 0;
            }
            catch (RecoverableError e)
            {
                output << "Error: " << e.what() << std::endl;
            }
            catch (...)
            {
                // TODO: Do we really want to catch all exceptions here?
                // Seems we want to at least print out the error message for BitFunnel exceptions.

                output << "Unexpected error.";
            }
        }

        return returnCode;
    }


    void PerfTool::Advice(std::ostream& output) const
    {
        output
            << "Index failed to load." << std::endl
            << std::endl
            << "Verify that directory path is valid and that the folder contains index files." << std::endl
            << "You can generate new index files with" << std::endl
            << "  BitFunnel statistics <manifest> <directory> -statistics" << std::endl
            << "  BitFunnel termtabe <directory>" << std::endl
            << "For more information run \"BitFunnel statistics -help\" and" << std::endl
            << "\"BitFunnel termtable -help\"." << std::endl;
    }

    void unknownExceptionHandler(std::exception_ptr eptr)
    {
        try
        {
            if (eptr)
            {
                std::rethrow_exception(eptr);
            }
        }
        catch(const std::exception & e)
        {
            std::cout << "Caught unknown exception \"" << e.what() << "\"\n";
            throw;
        }
    }

    void PerfTool::Go(std::istream& /*input*/,
                  std::ostream& output,
                  char const * directory,
                  size_t gramSize,
                  size_t threadCount) const
    {
        output
            << "BitFunnel perf suite" << std::endl
            << "Starting " << threadCount
            << " thread" << ((threadCount == 1) ? "" : "s") << std::endl
            << "(plus one extra thread for the Recycler.)" << std::endl
            << std::endl
            << "directory = \"" << directory << "\"" << std::endl
            << "gram size = " << gramSize << std::endl
            << std::endl;

        Environment environment(m_fileSystem,
                                directory,
                                gramSize,
                                threadCount);

        output
            << "Starting index ..."
            << std::endl;

        try
        {
            environment.StartIndex();
        }
        catch (...)
        {
            Advice(output);
            throw;
        }

        output
            << "Index started successfully."
            << std::endl;

        // Configure the index.
        IFileSystem & fileSystem =
            environment.GetFileSystem();
        TaskPool & taskPool = environment.GetTaskPool();
        IConfiguration const & configuration =
            environment.GetConfiguration();
        IIngestor & ingestor = environment.GetIngestor();

        // Ingest all files in the manifest.
        std::exception_ptr eptr;
        try
        {
            output
                << "Ingesting files."
                << std::endl;

            auto manifest = Factories::CreateChunkManifestIngestor(
                fileSystem,
                fileNames,
                configuration,
                ingestor,
                true);

            auto start = Time::now();
            IngestChunks(*manifest, threadCount);
            auto end = Time::now();

            ms d = std::chrono::duration_cast<ms>(end - start);
            output << "Ingestion took " << d.count() << " ms" << std::endl;
        }
        catch (RecoverableError e)
        {
            output << "Error: " << e.what() << std::endl;
        }
        catch (...)
        {
            eptr = std::current_exception();
        }
        unknownExceptionHandler(eptr);
        eptr = nullptr;

        // Execute queries.
        try
        {
            const size_t numQueries = 1;

            // Generate the query tree.
            // std::string command = "wings";
            std::string command = "dskljfdsjkdsjklsdljksdjkl";
            auto streamConfiguration = Factories::CreateStreamConfiguration();
            auto diagnosticStream = Factories::CreateDiagnosticStream(output);
            QueryPipeline pipeline(*streamConfiguration);

            std::vector<std::vector<DocId>> results(numQueries);

            // Evaluation of end-to-end query scanning, from parsing to
            // evaluation.
            auto start = Time::now();
            for (size_t i = 0; i < numQueries; i++)
            {
                auto tree = pipeline.ParseQuery(command.c_str());

                // Evaluate query.
                auto observed = Factories::RunSimplePlanner(
                    *tree,
                    environment.GetSimpleIndex(),
                    *diagnosticStream);
                results[i] = observed;
            }
            auto end = Time::now();

            ms d = std::chrono::duration_cast<ms>(end - start);
            output
                << "Average query latency over " << numQueries << ": "
                << static_cast<double>(d.count()) / numQueries << " ms"
                << std::endl;

            output << "Verifying queries." << std::endl;

            // Verify queries.
            size_t falsePositives = 0;
            size_t falseNegatives = 0;
            size_t observed = 0;
            size_t expected = 0;
            auto & cache = environment.GetIngestor().GetDocumentCache();
            TermMatchTreeEvaluator evaluator(configuration);
            for (size_t i = 0; i < numQueries; i++)
            {
                auto verifier = Factories::CreateMatchVerifier();
                size_t matchCount = 0;
                size_t documentCount = 0;

                for (auto entry : cache)
                {
                    ++documentCount;
                    auto tree = pipeline.ParseQuery(command.c_str());
                    bool matches = evaluator.Evaluate(*tree, entry.first);

                    if (matches)
                    {
                        ++matchCount;
                        verifier->AddExpected(entry.second);
                    }
                }

                for (auto docId : results[i])
                {
                    verifier->AddObserved(docId);
                }
                verifier->Verify();
                falsePositives += verifier->m_falsePositives.size();
                falseNegatives += verifier->m_falseNegatives.size();
                observed += verifier->m_observed.size();
                expected += verifier->m_expected.size();
            }

            output
                << "Ratio of (false positives)/(total) over all matches: "
                << static_cast<double>(falsePositives) /
                   static_cast<double>(observed)
                << std::endl
                << "False positives: " << falsePositives << std::endl
                << "Observed: " << observed << std::endl
                << "Expected: " << expected << std::endl
                << "False negatives: " << falseNegatives << std::endl;
        }
        catch (...)
        {
            eptr = std::current_exception();
        }
        unknownExceptionHandler(eptr);

        taskPool.Shutdown();
    }
}
