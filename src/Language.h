#pragma once

enum struct Strings : u32 {
    NewGame = 0,
    GatesGame,
    Exit,
    Language,
    CurrentLanguage,
    _Count
};

enum struct Language : u32 {
    English, Russian
};

const char16* GetString(Strings id);

void InitLanguageRussian();
void InitLanguageEnglish();
