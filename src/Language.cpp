#include "Language.h"

const char32* GetString(Strings id) {
    auto context = GetContext();
    const char32* result = context->strings[(u32)id];
    return result;
}

void InitLanguageRussian() {
    auto context = GetContext();

    context->strings[(u32)Strings::CurrentLanguage] = U"Ðóññêèé";

    context->strings[(u32)Strings::NewGame] = U"Íîâàÿ Èãðà";
    context->strings[(u32)Strings::GatesGame] = U"ÂÎÐÎÒÀ ÈÃÐÀ";
    context->strings[(u32)Strings::Exit] = U"Âûõîä";
    context->strings[(u32)Strings::Language] = U"ßçûê";
}

void InitLanguageEnglish() {
    auto context = GetContext();

    context->strings[(u32)Strings::CurrentLanguage] = U"English";

    context->strings[(u32)Strings::NewGame] = U"New Game";
    context->strings[(u32)Strings::GatesGame] = U"GATES GAME";
    context->strings[(u32)Strings::Exit] = U"Exit";
    context->strings[(u32)Strings::Language] = U"Language";
}
