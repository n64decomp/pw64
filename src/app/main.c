#include "common.h"
#include <uv_graphics.h>
#include <uv_system.h>
#include "demo.h"
#include "game.h"

void appEntrypoint(void) {
    uvSysInit(0);
    uvGfxEnableGamma(0);
    gameInit();
    while (demo_80323020() != 0) {
        gameUpdate();
    }
    uvSysUnknownStub(0);
}
