#pragma once

#include "ExceptionClause.h"

namespace Drill4dotNet
{
    // Represents one section with definitions of several
    // try-catch or try-finally blocks.
    // Can be constructed from .net api structures and
    // converted back to them.
    // Reference: ECMA-335, Common Language Infrastructure,
    // part II.25.4.5 Method data section
    // https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf
    class ExceptionsSection
    {
    private:
        // Value indicating whether more sections present in method
        // after this section.
        bool m_hasMoreSections;

        // Value indicating whether this instance was constructed from
        // a fat section header.
        bool m_fat;

        // Try-catch and try-finally clauses stored in this section.
        std::vector<ExceptionClause> m_clauses;
    public:

        // Fills a new instance from method bytes representation.
        // @param source : the method bytes, will be advanced by the length
        //     of the section.
        // @param sourceEnd : the end of the method bytes.
        // @param target : the instructions stream to add new labels to.
        // @param labelCreator : the tool to emit new labels.
        ExceptionsSection(
            std::vector<std::byte>::const_iterator& source,
            const std::vector<std::byte>::const_iterator sourceEnd,
            InstructionStream& target,
            LabelCreator& labelCreator);

        // Serializes this instance as bytes and adds them to the
        // given vector.
        // @param target : will append serialized bytes there.
        void AppendToBytes(std::vector<std::byte>& target) const;

        // Value indicating whether more sections present in method
        // after this section.
        constexpr bool HasMoreSections() const noexcept
        {
            return m_hasMoreSections;
        }

        // Try-catch and try-finally clauses stored in this section.
        constexpr const std::vector<ExceptionClause>& Clauses() const noexcept
        {
            return m_clauses;
        }
    };
}
