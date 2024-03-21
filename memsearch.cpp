#include "pch.h"

#include "memsearch.h"
#include <Psapi.h>


static std::tuple<LPVOID, size_t> GetModuleBaseAndSize(HMODULE hModule)
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

Lookup::Lookup(const wchar_t *module)
{
    MODULEINFO mi;
    auto hModule = GetModuleHandleW(module);
    if (hModule != NULL
        && GetModuleInformation(GetCurrentProcess(), hModule, &mi, sizeof(mi)) != 0)
    {
        base_ = (byte*)mi.lpBaseOfDll;
        baseLength_ = mi.SizeOfImage;

        auto [base, size] = GetModuleBaseAndSize(hModule);
        assert(base == base_);
        assert(size == baseLength_);
    }
    else
    {
        //throw std::system_error(GetLastError(), std::system_category(), "GetModuleInformation failed");
        var_debug(__func__, "GetModuleInformation failed with", GetLastError());
    }

    if (module != nullptr)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> uconv;
        var_debug(__func__, uconv.to_bytes(module), (void *)base_, ':', baseLength_);
    }
    else
    {
        var_debug(__func__, "executable", (void*)base_, ':', baseLength_);
    }
}
