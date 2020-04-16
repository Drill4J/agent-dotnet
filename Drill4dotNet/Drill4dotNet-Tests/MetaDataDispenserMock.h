#pragma once

#include <string>
#include <functional>

#include <gmock/gmock.h>

#include "IMetadataDispenser.h"
#include "MetaDataAssemblyImportMock.h"
#include "MetadataImportMock.h"

namespace Drill4dotNet
{
    class MetaDataDispenserMock
    {
    public:

        MetaDataDispenserMock(const TrivialLogger&)
        {
        }

        MetaDataAssemblyImportMock OpenScopeMetaDataAssemblyImport(const std::filesystem::path& path, const TrivialLogger logger) const
        {
            return { logger };
        }

        std::optional<MetaDataAssemblyImportMock> TryOpenScopeMetaDataAssemblyImport(const std::filesystem::path& path, const TrivialLogger logger) const
        {
            return { logger };
        }

        MetadataImportMock OpenScopeMetaDataImport(const std::filesystem::path& path, const TrivialLogger logger) const
        {
            return { logger };
        }

        std::optional<MetadataImportMock> TryOpenScopeMetaDataImport(const std::filesystem::path& path, const TrivialLogger logger) const
        {
            return { logger };
        }
    };

    static_assert(IsMetadataDispenser<MetaDataDispenserMock>);
}
