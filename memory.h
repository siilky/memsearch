#pragma once

using byte = unsigned char;


// returns pointer to start of data
const byte* findData(const byte *data, unsigned dataSize, const byte *start, unsigned length, unsigned startOffset = 0);

template <unsigned dataSize>
inline const byte * findData(const byte(&data)[dataSize], const byte *start, unsigned length, unsigned startOffset = 0)
{
    return findData(data, dataSize, start, length, startOffset);
}


const byte * findDataR(const byte *data, unsigned dataSize, const byte *end, unsigned length);

template <unsigned dataSize>
inline const byte * findDataR(const byte(&data)[dataSize], const byte *end, unsigned length)
{
    return findDataR(data, dataSize, end, length);
}


const byte * findPattern(const unsigned short *pattern, unsigned patternLength, const byte *start, unsigned length, unsigned startOffset = 0);

template <unsigned patternLength>
inline const byte * findPattern(const unsigned short (&pattern)[patternLength], const byte *start, unsigned length, unsigned startOffset = 0)
{
    return findPattern(pattern, patternLength, start, length, startOffset);
}
