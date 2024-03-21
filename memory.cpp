#include "pch.h"

#include "memory.h"


const byte * findData(const byte * data, unsigned dataSize, const byte *start, unsigned length, unsigned startOffset)
{
    for (const byte *ptr = start + startOffset; ptr <= (start + length - dataSize); ++ptr)
    {
        if (memcmp(data, ptr, dataSize) == 0)
        {
            return ptr;
        }
    }
    return nullptr;
}

const byte * findDataR(const byte *data, unsigned dataSize, const byte *end, unsigned length)
{
    for (const byte *ptr = end - dataSize; ptr >= (end - length); --ptr)
    {
        if (memcmp(data, ptr, dataSize) == 0)
        {
            return ptr;
        }
    }
    return nullptr;
}

static inline bool matchPattern(const unsigned short *pattern, unsigned patternLength, const byte *ptr)
{
    while (patternLength > 0
           && ((((*pattern) & 0xFF00) != 0)
               || byte((*pattern) & 0x00FF) == *ptr))
    {
        ++ptr;
        ++pattern;
        --patternLength;
    }
    return patternLength == 0;
}

const byte * findPattern(const unsigned short *pattern, unsigned patternLength, const byte *start, unsigned length, unsigned startOffset)
{
    assert((startOffset + patternLength) < length);

    for (const byte *ptr = start + startOffset; ptr <= (start + length - patternLength); ++ptr)
    {
        if (matchPattern(pattern, patternLength, ptr))
        {
            return ptr;
        }
    }

    return nullptr;
}
