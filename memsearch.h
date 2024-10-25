#pragma once

#include <Psapi.h>

#include "memory.h"
/*
#if !defined(VAR_LOG)
template<typename T, typename... Args>
void printArgs(T first, Args... args)
{
    std::cout << first << " ";
    printArgs(args...);
}
else

#endif
*/

#if defined(MEMSEARCH_SPDLOG)
    #include "spdlog/spdlog.h"
    template<typename Char = char, typename... Args>
    void var_debug(Args... args)
    {
        std::basic_ostringstream<Char> os;
        ((os << args << ' '), ...);
        spdlog::debug(os.str());
    }
#elif defined(MEMSEARCH_ELPP)
    template<typename Char, typename... Args>
    void var_debug(std::basic_ostream<Char> & os, Args... args)
    {
        ((os << args << ' '), ...) << std::endl;
    }
#else
    #include <iostream>
    template<typename... Args>
    void var_debug(Args... args)
    {
        ((std::cout << args << ' '), ...) << std::endl;
    }
#endif

std::tuple<LPVOID, size_t> GetModuleBaseAndSize(HMODULE hModule);

class Lookup
{
public:
    explicit Lookup(const wchar_t * module = nullptr)
    {
        MODULEINFO mi;
        auto hModule = GetModuleHandleW(module);
        if (hModule != NULL
            && GetModuleInformation(GetCurrentProcess(), hModule, &mi, sizeof(mi)) != 0)
        {
            base_ = (byte *)mi.lpBaseOfDll;
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
#if defined(MEMSEARCH_SPDLOG) || defined(MEMSEARCH_ELPP)
            //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> uconv;
            var_debug(__func__, module, (void *)base_, ':', baseLength_);
#endif
        }
        else
        {
            var_debug(__func__, "executable", (void *)base_, ':', baseLength_);
        }
    }
    virtual ~Lookup() = default;

    const byte * base() const
    {
        return base_;
    }

    uintptr_t calcVA(uintptr_t addr) const
    {
        return (addr - (uintptr_t)base_) + defaultVA;
    }

    bool isNull() const
    {
        return base_ == nullptr;
    }

    bool isValid() const
    {
        return pointer_ != nullptr;
    }

    void valid()    // set to any 'valid' value
    {
        pointer_ = (const byte *)1;
    }

    const byte * pointer() const
    {
        return pointer_;
    }
    const uintptr_t address() const
    {
        return (uintptr_t)pointer_;
    }

    Lookup & getRelAddr(uintptr_t & ra, int offset = 0)
    {
        if (isValid())
        {
            auto ptr = pointer_ + offset;
            ra = (uintptr_t)(ptr + *(unsigned *)ptr + sizeof(ptr));
        }
        return *this;
    }

    template<typename T>
    Lookup & getData(T & ra, int offset = 0)
    {
        if (isValid())
        {
            ra = *(T *)(pointer_ + offset);
        }
        return *this;
    }


    template<unsigned Size>
    Lookup & string(const char(&str)[Size], unsigned va = defaultVA)
    {
        assert(va >= defaultVA);
        if (isNull())
        {
            pointer_ = nullptr;
            return *this;
        }
        lastRefSize_ = Size;
        pointer_ = findData((const byte *)str, Size, base_, baseLength_, va - defaultVA);
        var_debug(__func__, dva(pointer_));
        return *this;
    }

    template<unsigned Size>
    Lookup & thenString(const char(&str)[Size], unsigned lookSize)
    {
        if (isValid())
        {
            lastRefSize_ = Size;

            pointer_ = findData((const byte *)str, Size, pointer_, lookSize);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }


    template<unsigned Size>
    Lookup & wstring(const wchar_t(&str)[Size], unsigned va = defaultVA)
    {
        if (isNull())
        {
            pointer_ = nullptr;
            return *this;
        }
        lastRefSize_ = Size;
        pointer_ = findData((const byte *)str, Size * sizeof(wchar_t), base_, baseLength_, va - defaultVA);
        var_debug(__func__, dva(pointer_));
        return *this;
    }

    // returns pointer to start of data
    template<unsigned Size>
    Lookup & data(const byte(&data)[Size], unsigned va = defaultVA)
    {
        assert(va >= defaultVA);
        if (isNull())
        {
            pointer_ = nullptr;
            return *this;
        }
        lastRefSize_ = Size;
        pointer_ = findData(data, Size, base_, baseLength_, va - defaultVA);
        var_debug(__func__, dva(pointer_));
        return *this;
    }

    template<unsigned Size>
    Lookup & thenData(const byte(&data)[Size], unsigned lookSize)
    {
        if (isValid())
        {
            lastRefSize_ = Size;

            pointer_ = findData(data, Size, pointer_, lookSize);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    template<unsigned Size>
    Lookup & thenDataBack(const byte(&data)[Size], unsigned lookSize)
    {
        if (isValid())
        {
            lastRefSize_ = Size;

            pointer_ = findDataR(data, Size, pointer_, lookSize);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    template<unsigned Size>
    Lookup & pattern(const unsigned short(&data)[Size], unsigned va = defaultVA)
    {
        assert(va >= defaultVA);
        if (isNull())
        {
            pointer_ = nullptr;
            return *this;
        }
        lastRefSize_ = Size;
        pointer_ = findPattern(data, Size, base_, baseLength_, va - defaultVA);
        var_debug(__func__, dva(pointer_));
        return *this;
    }

    template<unsigned Size>
    Lookup & thenPattern(const unsigned short(&data)[Size], unsigned lookSize)
    {
        if (isValid())
        {
            lastRefSize_ = Size;
            pointer_ = findPattern(data, Size, pointer_, lookSize);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    template<unsigned Size>
    Lookup & thenPatternBack(const unsigned short(&data)[Size], unsigned lookSize)
    {
        if (isValid())
        {
            lastRefSize_ = Size;
            // a little trick (shift start instead of doing backsearch)
            pointer_ = findPattern(data, Size, pointer_ - lookSize, lookSize);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    template<unsigned Size>
    Lookup & referencedAt(byte(&data)[Size], unsigned refOffset, unsigned va = defaultVA)
    {
        assert(Size > 4 && refOffset + 4 <= Size);
        assert(va >= defaultVA);
        if (isValid())
        {
            lastRefSize_ = Size;

            *(const byte **)(data + refOffset) = pointer_;
            pointer_ = findData(data, Size, base_, baseLength_, va - defaultVA);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    template<unsigned Size>
    Lookup & referencedAtPattern(unsigned short(&pattern)[Size], unsigned refOffset, unsigned va = defaultVA)
    {
        assert(Size > 4 && refOffset + 4 <= Size);
        assert(va >= defaultVA);
        if (pointer_ != nullptr)
        {
            lastRefSize_ = Size;

            pattern[refOffset + 0] = (unsigned short)(((uintptr_t)pointer_ >> 0) & 0xFF);
            pattern[refOffset + 1] = (unsigned short)(((uintptr_t)pointer_ >> 8) & 0xFF);
            pattern[refOffset + 2] = (unsigned short)(((uintptr_t)pointer_ >> 16) & 0xFF);
            pattern[refOffset + 3] = (unsigned short)(((uintptr_t)pointer_ >> 24) & 0xFF);
            pointer_ = findPattern(pattern, Size, base_, baseLength_, va - defaultVA);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    template<unsigned Size>
    Lookup & referencedAsOffset(const byte(&data)[Size], unsigned va = defaultVA)
    {
        assert(va >= defaultVA);
        if (isValid())
        {
            lastRefSize_ = Size;

            const byte * p = base_ + (va - defaultVA);
            while (p != nullptr && p < (base_ + baseLength_ - sizeof(uintptr_t)))
            {
                p = findData(data, Size, p, base_ + baseLength_ - p);
                if (p != nullptr)
                {
                    uintptr_t vtRef = pointer_ - (p + Size + sizeof(vtRef));
                    if (vtRef == *(unsigned *)(p + Size))
                    {
                        pointer_ = p;
                        var_debug(__func__, dva(pointer_));
                        return *this;
                    }
                    else
                    {
                        ++p;
                    }
                }
            }
            pointer_ = nullptr;
        }
        return *this;
    }

    Lookup & referenced(unsigned va = defaultVA)
    {
        assert(va >= defaultVA);
        if (isValid())
        {
            lastRefSize_ = sizeof(pointer_);

            pointer_ = findData((const byte *)&pointer_, sizeof(pointer_), base_, baseLength_, va - defaultVA);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    inline Lookup & pointerTo()
    {
        if (isValid())
        {
            pointer_ = *(const byte **)(pointer_);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    inline Lookup & relPointerTo()
    {
        if (isValid())
        {
            pointer_ = pointer_ + *(unsigned *)pointer_ + sizeof(pointer_);
            var_debug(__func__, dva(pointer_));
        }
        return *this;
    }

    inline Lookup & after()
    {
        if (isValid() && lastRefSize_ > 0)
        {
            pointer_ += lastRefSize_;
        }
        return *this;
    }

    inline Lookup & from(int offset)
    {
        if (isValid())
        {
            pointer_ += offset;
        }
        return *this;
    }

private:
    inline const void * dva(const byte * p) const
    {
        return p ? (byte *)(p - base_) + defaultVA : 0;
    }

    static const unsigned   defaultVA = 0x00400000;

    const byte * base_ = nullptr;
    unsigned        baseLength_ = 0;

    const byte * pointer_ = nullptr;
    unsigned        lastRefSize_ = 0;
};
