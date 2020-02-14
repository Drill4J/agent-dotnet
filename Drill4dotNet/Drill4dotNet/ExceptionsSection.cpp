#include "pch.h"
#include "ExceptionsSection.h"

#include <algorithm>

namespace Drill4dotNet
{
    template<typename TSection, typename TClause>
    void ReadClauses(
        std::vector<std::byte>::const_iterator& source,
        const std::vector<std::byte>::const_iterator sourceEnd,
        InstructionStream& target,
        LabelCreator& labelCreator,
        std::vector<ExceptionClause>& clauses)
    {
        constexpr size_t clauseSize = sizeof(TClause);

        // same for SMALL and FAT, SMALL has padding for equal header sizes
        constexpr size_t headerSize = sizeof(IMAGE_COR_ILMETHOD_SECT_FAT);

        if (sourceEnd - source < headerSize)
        {
            throw std::runtime_error("Unexpected end of the input in the middle of exception section header");
        }

        const TSection& sectionHeader = *(reinterpret_cast<const TSection*>(&*source));

        const size_t clausesCount = (sectionHeader.DataSize - headerSize) / clauseSize;
        if (clausesCount * clauseSize + headerSize != sectionHeader.DataSize)
        {
            throw std::runtime_error("Unexpected size of exception section: it does not hold a whole number of exception clauses");
        }

        if (sourceEnd - source < sectionHeader.DataSize)
        {
            throw std::runtime_error("Unexpected end of the input in the middle of exception section clauses");
        }

        clauses.reserve(clausesCount);
        source += headerSize;
        for (size_t i = 0; i != clausesCount; ++i)
        {
            const TClause& clause = *(reinterpret_cast<const TClause*>(&*source));
            source += clauseSize;
            clauses.emplace_back(
                clause,
                target,
                labelCreator);
        }
    }

    ExceptionsSection::ExceptionsSection(
        std::vector<std::byte>::const_iterator& source,
        const std::vector<std::byte>::const_iterator sourceEnd,
        InstructionStream& target,
        LabelCreator& labelCreator)
    {
        if (source == sourceEnd)
        {
            throw std::runtime_error("Unexpected end of the input: no exception section header provided");
        }

        std::byte firstByte { *source };
        if ((firstByte & std::byte { CorILMethodSect::CorILMethod_Sect_EHTable }) == std::byte{ 0 })
        {
            throw std::runtime_error("The data section is not an exception handling section");
        }

        m_fat = (firstByte & std::byte { CorILMethodSect::CorILMethod_Sect_FatFormat }) != std::byte{ 0 };
        m_hasMoreSections = (firstByte & std::byte { CorILMethodSect::CorILMethod_Sect_MoreSects }) != std::byte{ 0 };
        if ((firstByte & ~(std::byte{ CorILMethodSect::CorILMethod_Sect_FatFormat }
            | std::byte{ CorILMethodSect::CorILMethod_Sect_MoreSects }
            | std::byte{ CorILMethodSect::CorILMethod_Sect_EHTable })) != std::byte{ 0 })
        {
            throw std::runtime_error("Unknown section flags detected");
        }

        if (m_fat)
        {
            ReadClauses<IMAGE_COR_ILMETHOD_SECT_FAT, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT>(
                source,
                sourceEnd,
                target,
                labelCreator,
                m_clauses);
        }
        else
        {
            ReadClauses<IMAGE_COR_ILMETHOD_SECT_SMALL, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL>(
                source,
                sourceEnd,
                target,
                labelCreator,
                m_clauses);
        }
    }

    void ExceptionsSection::AppendToBytes(std::vector<std::byte>& target) const
    {
        CorILMethodSect flags{ CorILMethodSect::CorILMethod_Sect_EHTable };
        bool shouldBeFat = m_fat || std::any_of(
            m_clauses.cbegin(),
            m_clauses.cend(),
            [](const ExceptionClause& c) { return !c.CanPutToSmallHeader(); });

        if (shouldBeFat)
        {
            flags = static_cast<CorILMethodSect>(flags | CorILMethodSect::CorILMethod_Sect_FatFormat);
        }

        if (m_hasMoreSections)
        {
            flags = static_cast<CorILMethodSect>(flags | CorILMethodSect::CorILMethod_Sect_MoreSects);
        }

        // same for SMALL and FAT, SMALL has padding for equal header sizes
        constexpr size_t headerSize = sizeof(IMAGE_COR_ILMETHOD_SECT_FAT);
        if (shouldBeFat)
        {
            IMAGE_COR_ILMETHOD_SECT_FAT header;
            header.Kind = flags;
            header.DataSize = headerSize + sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT) * m_clauses.size();
            AppendAsBytes(target, header);
            for (size_t i = 0; i != m_clauses.size(); ++i)
            {
                IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT clause = m_clauses[i].FillFatHeader();
                AppendAsBytes(target, clause);
            }
        }
        else
        {
            IMAGE_COR_ILMETHOD_SECT_SMALL header;
            header.Kind = flags;
            header.DataSize = headerSize + sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL) * m_clauses.size();
            AppendAsBytes(target, header);
            AppendAsBytes(target, decltype(std::declval<IMAGE_COR_ILMETHOD_SECT_EH_SMALL>().Reserved) { 0 });
            for (size_t i = 0; i != m_clauses.size(); ++i)
            {
                IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL clause = m_clauses[i].FillSmallHeader();
                AppendAsBytes(target, clause);
            }
        }
    }
}
