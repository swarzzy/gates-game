#pragma once

struct GameContext;
struct Console;
struct ConsoleCommandArgs;

void ConsoleDrawModeCommand(Console* console, GameContext* gameContext, ConsoleCommandArgs* args);
void SaveAsConsoleCommand(Console* console, GameContext* gameContext, ConsoleCommandArgs* args);
void LoadConsoleCommand(Console* console, GameContext* gameContext, ConsoleCommandArgs* args);
