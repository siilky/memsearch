#include "pch.h"

#include "memsearch.h"


std::tuple<LPVOID, size_t> GetModuleBaseAndSize(HMODULE hModule)
{
    MEMORY_BASIC_INFORMATION mbi;

    if (!VirtualQuery(hModule, &mbi, sizeof(mbi)))
        return std::make_tuple(nullptr, 0);

    auto base = mbi.AllocationBase;

    MEMORY_BASIC_INFORMATION mbiNext;
    while (VirtualQuery((LPBYTE)mbi.BaseAddress + mbi.RegionSize, &mbiNext, sizeof(mbiNext)))
    {
        if (mbi.AllocationBase != mbiNext.AllocationBase)
            break;

        mbi = mbiNext;
    }

    auto size = (SIZE_T)((LPBYTE)mbi.BaseAddress + mbi.RegionSize - (LPBYTE)base);

    return std::make_tuple(base, size);
}
