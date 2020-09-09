#include "LinuxCodeLoader.h"
#include <stdlib.h>

#include <dlfcn.h>

static void __cdecl GameUpdateAndRenderDummy(PlatformState*, GameInvoke, void**) {

}

void UnloadGameCode(LibraryData* lib) {
    if (lib->handle) {
        dlclose(lib->handle);
    }
    lib->handle = 0;
    lib->GameUpdateAndRender = GameUpdateAndRenderDummy;
    remove(LibraryData::TempLibName);
}

b32 UpdateGameCode(LibraryData* lib) {
    b32 updated = false;
    struct stat fileAttribs;
    if (stat(LibraryData::LibName, &fileAttribs) == 0) {
        time_t writeTime = fileAttribs.st_mtime;
        if (writeTime != lib->lastChangeTime) {
            UnloadGameCode(lib);

            auto copied = DebugCopyFile(LibraryData::LibName, LibraryData::TempLibName, false);
            if (copied) {
                lib->handle = dlopen(LibraryData::TempLibName, RTLD_LAZY | RTLD_LOCAL);
                if (lib->handle) {
                    GameUpdateAndRenderFn* gameUpdateAndRender = (GameUpdateAndRenderFn*)dlsym(lib->handle, "GameUpdateAndRender");
                    if (gameUpdateAndRender) {
                        lib->GameUpdateAndRender = gameUpdateAndRender;
                        updated = true;
                        lib->lastChangeTime = writeTime;
                    } else {
                        log_print("[Error] Failed to get GameUpdateAndRender() address.\n");
                    }
                } else {
                    log_print("[Error] Failed to load game library.\n");
                }
            }
        }
    }
    return updated;
}
