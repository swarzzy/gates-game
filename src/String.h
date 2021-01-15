#pragma once

template <typename Char>
constexpr usize StringSizeZ(const Char* str) {
    usize result = 0;
    while (*str) {
        result++;
        str++;
    }
    result++;
    result *= sizeof(Char);
    return result;
}


constexpr usize StringSize(const char* str) {
    usize result = 0;
    while (*str) {
        result++;
        str++;
    }
    return result;
}


template <typename Char>
constexpr usize StringLengthZ(const Char* str) {
    usize result = 0;
    while (*str) {
        result++;
        str++;
    }
    result++;
    return result;
}

template <typename Char>
constexpr usize StringLength(const Char* str) {
    usize result = 0;
    while (*str) {
        result++;
        str++;
    }
    return result;
}

#define CopyStringLiteral(str, allocator) CopyString(str, sizeof(str), allocator)

template <typename Char>
Char* CopyString(const Char* str, usize size, Allocator* allocator) {
    Char* buffer = allocator->Alloc<Char>(size, false);
    memcpy(buffer, str, sizeof(Char) * size);
    return buffer;
}

template <typename Char>
Char* CopyString(const Char* str, Allocator* allocator) {
    Char* result = CopyString(str, StringSizeZ(str), allocator);
    return result;
}

bool MatchStrings(const char* a, const char* b) {
    bool result = true;
    while(*a) {
        if (!(*b)) {
            result = false;
            break;
        }
        if (*a != *b) {
            result = false;
            break;
        }
        a++;
        b++;
    }
    return result;
}

bool StringsAreEqual(const char* a, const char* b) {
    bool result = true;
    while(*a) {
        if (!(*b)) {
            result = false;
            break;
        }
        if (*a != *b) {
            result = false;
            break;
        }
        a++;
        b++;
    }
    if (*b) {
        result = false;
    }
    return result;
}

bool IsSpace(char c) {
    bool result = false;
    if (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v') {
        result = true;
    }
    return result;
}

bool IsSpace(char16 c) {
    bool result = false;
    if (c == u' ' || c == u'\f' || c == u'\n' || c == u'\r' || c == u'\t' || c == u'\v') {
        result = true;
    }
    return result;
}

bool IsSpace(char32 c) {
    bool result = false;
    if (c == U' ' || c == U'\f' || c == U'\n' || c == U'\r' || c == U'\t' || c == U'\v') {
        result = true;
    }
    return result;
}

Option<f32> StringToF32(const char* string) {
    Option<f32> result  = {};
    char* end;
    // Is that crazy global variable error checking even thread safe?
    errno = 0;
    f32 value = strtof(string, &end);
    // TODO: At some point I should stop using this crazy CRT crappiness and write my own routine
    if ((end != string) && (value != HUGE_VAL) && (value != HUGE_VALF) && (value != HUGE_VALL) && (errno == 0)) {
        result = Option(value);
    }
    return result;
}

Option<i32> StringToI32(const char* string) {
    Option<i32> result  = {};
    char* end;
    // Is that crazy global variable error checking even thread safe?
    errno = 0;
    i32 value = (i32)strtol(string, &end, 10);
    // TODO: At some point I should stop using this crazy CRT crappiness and write my own routine
    if ((end != string) && (value != LONG_MAX) && (value != LONG_MIN) && (value != HUGE_VALL) && (errno == 0)) {
        result = Option(value);
    }

    return result;
}

Option<u32> StringToU32(const char* string) {
    Option<u32> result  = {};
    char* end;
    // Is that crazy global variable error checking even thread safe?
    errno = 0;
    u32 value = (u32)strtoul(string, &end, 10);
    // TODO: At some point I should stop using this crazy CRT crappiness and write my own routine
    if ((end != string) && (value != ULONG_MAX) && (errno == 0)) {
        result = Option(value);
        result.value = value;
    }
    return result;
}


bool PrettySize(char* buffer, u32 bufferSize, uptr bytes) {
    bool result = false;
    f64 size = (f64)bytes;
    const char* suffixes[4] = {"bytes", "kb", "mb", "gb"};
    i32 s = 0;
    while (size >= 1024 && s < 4)
    {
        s++;
        size /= 1024;
    }
    if (size - Floor(size) == 0.0) {
        u32 requiredSize = (u32)snprintf(nullptr, 0, "%ld %s", (long)size, suffixes[s]) + 1;
        if (requiredSize <= bufferSize) {
            sprintf(buffer, "%ld %s", (long)size, suffixes[s]);
            result = true;
        }
    }
    else {
        u32 requiredSize = (u32)snprintf(nullptr, 0, "%.1f %s", size, suffixes[s]) + 1;
        if (requiredSize <= bufferSize) {
            sprintf(buffer, "%.1f %s", size, suffixes[s]);
            result = true;
        }
    }
    return result;
}

struct SplitFilePathResult {
    char* directory;
    char* filename;
};

// TODO: It has a bug somwhere. See todo for flux engine
SplitFilePathResult SplitFilePath(char* path) {
    SplitFilePathResult result {};
    auto pathLength = (uptr)strlen(path);
    uptr splitPos = 0;
    for (uptr i = pathLength - 1; i > 0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            splitPos = i;
            break;
        }
    }
    if (splitPos == pathLength - 1) {
        result.directory = path;
    } else if (splitPos > 0) {
        path[splitPos] = 0;
        result.directory = path;
        result.filename = path + (splitPos + 1);
    } else {
        result.filename = path + 1;
    }
    return result;
}

i32 FindLastDirSeparator(const char* path) {
    auto pathLength = (i32)strlen(path);
    i32 splitPos = -1;
    for (i32 i = pathLength - 1; i > 0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            splitPos = i;
            break;
        }
    }
    return splitPos;
}

// [https://bjoern.hoehrmann.de/utf-8/decoder/dfa/]
// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const u8 utf8d[] = {
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
     8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
     0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
     0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
     0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
     1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
     1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
     1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

u32 Utf8Decode(u32* state, u32* codep, u32 byte) {
     u32 type = utf8d[byte];

     *codep = (*state != UTF8_ACCEPT) ?
         (byte & 0x3fu) | (*codep << 6) :
         (0xff >> type) & (byte);

     *state = utf8d[256 + *state*16 + type];
     return *state;
 }
