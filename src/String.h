#pragma once

template <typename Char, u32 Size> struct StaticString;
template <typename Char> struct DynamicString;

template <u32 Size>
using SString = StaticString<char, Size>;

using DString = DynamicString<char>;

template <typename Char, typename Derived>
struct StringBase {
};

template <typename Char, u32 Size>
struct StaticString : StringBase<Char, StaticString<Char, Size>> {
    Char data[Size];
    u32 at = nullptr;

    forceinline Char* Buffer() { return data; }
    forceinline u32 At() { return at; }
    forceinline u32 Capacity() { return Size; }

    StaticString();
};

template <typename Char>
struct DynamicString : StringBase<Char, DynamicString<Char>> {
    Char* data = nullptr;
    Allocator* allocator;

    forceinline Char* Buffer() { return data + sizeof(u16) * 2; }
    forceinline u32 At() { return *((u16*)data); }
    forceinline u32 Capacity() { return *(((u16*)data) + 1); }

    DynamicString() = default;
    DynamicString(Allocator* allocator);

    forceinline u16& _At() { return *((u16*)data); }
    forceinline u16& _Capacity() { return *(((u16*)data) + 1); }
};

#if 0
struct StaticString {
    char* data;
    u32 capacity;
    u32 at;

    StaticString(char* buff, u32 buffSize) {
        data = buff;
        capacity = buffSize;
        at = 0;
    }

    void Append(const char* str) {
        auto len = (usize)strlen(str);
        usize remainingSize = capacity - at - 1;
        usize sizeToCopy = Min(remainingSize, len);

        memcpy(data + at, str, sizeof(char) * sizeToCopy);
        at += sizeToCopy;
        assert(at < capacity);
        data[at] = 0;
    }

    void Appendfv(const char* fmt, va_list args) {
        i32 remainingSize = capacity - at - 1;
        if (remainingSize > 0) {
            assert(remainingSize > 0);
            i32 written = FormatStringV(data + at, remainingSize, fmt, args);
            assert(written >= 0);
            at += Min(remainingSize - 1, written);
            assert(at < capacity);
        }
    }


    void Appendf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        Appendfv(fmt, args);
        va_end(args);
    }

    const char* GetBuffer() const { return data; }
};
#endif


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

struct FloatParseResult {
    b32 succeed;
    f32 value;
};

FloatParseResult StringToFloat(const char* string) {
    FloatParseResult result  = {};
    char* end;
    // Is that crazy global variable error checking even thread safe?
    errno = 0;
    f32 value = strtof(string, &end);
    // TODO: At some point I should stop using this crazy CRT crappiness and write my own routine
    if ((end != string) && (value != HUGE_VAL) && (value != HUGE_VALF) && (value != HUGE_VALL) && (errno == 0)) {
        result.succeed = true;
        result.value = value;
    }
    return result;
}

struct IntParseResult {
    b32 succeed;
    i32 value;
};

IntParseResult StringToInt(const char* string) {
    IntParseResult result  = {};
    char* end;
    // Is that crazy global variable error checking even thread safe?
    errno = 0;
    i32 value = (i32)strtol(string, &end, 10);
    // TODO: At some point I should stop using this crazy CRT crappiness and write my own routine
    if ((end != string) && (value != LONG_MAX) && (value != LONG_MIN) && (value != HUGE_VALL) && (errno == 0)) {
        result.succeed = true;
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
