#include "Language.h"

const char16* GetString(Strings id) {
    auto context = GetContext();
    const char16* result = context->strings[(u32)id];
    return result;
}

void InitLanguageRussian() {
    auto context = GetContext();

    context->strings[(u32)Strings::CurrentLanguage] = u"Ðóññêèé";

    context->strings[(u32)Strings::NewGame] = u"Íîâàÿ Èãðà";
    context->strings[(u32)Strings::GatesGame] = u"ÂÎÐÎÒÀ ÈÃÐÀ";
    context->strings[(u32)Strings::Exit] = u"Âûõîä";
    context->strings[(u32)Strings::Language] = u"ßçûê";
}

void InitLanguageEnglish() {
    auto context = GetContext();

    context->strings[(u32)Strings::CurrentLanguage] = u"English";

    context->strings[(u32)Strings::NewGame] = u"New Game";
    context->strings[(u32)Strings::GatesGame] = u"GATES GAME";
    context->strings[(u32)Strings::Exit] = u"Exit";
    context->strings[(u32)Strings::Language] = u"Language";
}
