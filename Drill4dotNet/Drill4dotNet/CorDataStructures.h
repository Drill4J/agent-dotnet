#pragma once

#include "framework.h"

namespace Drill4dotNet
{
    struct FunctionInfo
    {
        ClassID classId;
        ModuleID moduleId;
        mdToken token;
    };
}
