#include <vector>

#include "BitFunnel/IInterface.h"
#include "BitFunnel/Index/Row.h"
// #include "FixedCapacityVector.h"

namespace BitFunnel
{
    class ICodeGenerator : public IInterface
    {
    public:
        typedef unsigned Label;

        // // RankDown compiler primitives
        // virtual void AndRow(unsigned id, bool inverted, unsigned rankDelta) = 0;
        // virtual void LoadRow(unsigned id, bool inverted, unsigned rankDelta) = 0;

        // virtual void LeftShiftOffset(unsigned shift) = 0;
        // virtual void RightShiftOffset(unsigned shift) = 0;
        // virtual void IncrementOffset() = 0;

        // virtual void Push() = 0;
        // virtual void Pop() = 0;

        // // Stack machine primitives
        // virtual void AndStack() = 0;
        // virtual void Constant(int value) = 0;
        // virtual void Not() = 0;
        // virtual void OrStack() = 0;
        // virtual void UpdateFlags() = 0;

        virtual void Report() = 0;

        // // Constrol flow primitives.
        // virtual Label AllocateLabel() = 0;
        // virtual void PlaceLabel(Label label) = 0;
        // virtual void Call(Label label) = 0;
        // virtual void Jmp(Label label) = 0;
        // virtual void Jnz(Label label) = 0;
        // virtual void Jz(Label label) = 0;
        // virtual void Return() = 0;
    };

    // TODO: make interface.
    class MatchInterpreter : public ICodeGenerator
    {
    public:
        typedef bool (*ReportCallback) (uint64_t accumulator, size_t index);

        void Run(ReportCallback report, Row* rows)
        {
            auto instruction = *m_ip;
            m_ip = instruction.Run();
        }

        //
        // ICodeGenerator methods.
        //
        virtual void Report() override;

    private:
        class Instruction
        {
        public:
            Instruction const * Run()
            {
                switch (m_opcode)
                {
                case Report:
                    return m_parameters.m_report.Run();
                    break;
                default:
                    break;
                    // TODO: error out.
                }
            }
        private:
            enum class Opcode
            {
                Report, Last
            };

            class Report
            {
            public:
                void Run();
            };

            union Parameters
            {
                Report m_report;
            };

            Opcode m_opcode;
            Parameters m_parameters;
        };
        // FixedCapacityVector<Instruction> m_code;
        std::vector<Instruction> m_code;
        Instruction* m_ip;
    };
}
