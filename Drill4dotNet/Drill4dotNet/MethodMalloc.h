#pragma once

#include "framework.h"
#include "ComWrapperBase.h"

namespace Drill4dotNet
{
    // Provides logging and error handling capabilities for IMethodMalloc.
    // TLogger: class with methods bool IsLogEnabled()
    //     and Log(). The second one should provide some
    //     object allowing to output data with << in the
    //     same manner as standard output streams do.
    template <typename TLogger>
    class MethodMalloc
    {
    private:
        ATL::CComQIPtr<IMethodMalloc> m_methodMalloc;
        TLogger m_logger;

        // Getting this from IMethodMalloc::Alloc will mean no space was found
        // at the address space of the target module.
        inline static const void* s_AllocFailed { (void*)E_OUTOFMEMORY };

    public:
        // Captures IMethodMalloc object allowing safe access to its methods.
        // methodMalloc: the IMethodMalloc object, which allows allocation memory
        //      for method bodies.
        MethodMalloc(
            ATL::CComQIPtr<IMethodMalloc> methodMalloc,
            TLogger logger)
            : m_logger(logger),
            m_methodMalloc(methodMalloc)
        {
        }

        // Tries to allocate a memory block of the given size.
        // Wraps IMethodMalloc::Alloc.
        // Returns std::nullopt if it could not find space in the target
        // module's address space.
        // @param size : the desired size of the resulting memory block.
        std::optional<void*> TryAlloc(uint32_t size)
        {
            void* const result = m_methodMalloc->Alloc(size);
            if (result == nullptr || result == s_AllocFailed)
            {
                if (m_logger.IsLogEnabled())
                {
                    m_logger.Log()
                        << L"MethodAlloc::TryAlloc failed. Could not allocate memory in the target module's address space.";
                }

                return std::nullopt;
            }

            return result;
        }

        // Allocates a memory block of the given size.
        // Wraps IMethodMalloc::Alloc.
        // Throws _com_error if it could not find space in the target
        // module's address space.
        // @param size : the desired size of the resulting memory block.
        void* Alloc(uint32_t size)
        {
            void* const result = m_methodMalloc->Alloc(size);
            if (result == nullptr || result == s_AllocFailed)
            {
                if (m_logger.IsLogEnabled())
                {
                    m_logger.Log()
                        << L"MethodAlloc::Alloc failed. Could not allocate memory in the target module's address space.";
                }

                throw _com_error(E_OUTOFMEMORY);
            }

            return result;
        }
    };
}
