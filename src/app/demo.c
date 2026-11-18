#include "common.h"
#include <uv_controller.h>
#include <uv_filesystem.h>
#include <uv_math.h>
#include <uv_memory.h>
#include <uv_utils.h>
#include "code_9A960.h"
#include "demo.h"
#include "demo_attitude.h"

typedef struct {
    s32 test;
    s32 unk4; // unused, but set in range [0-5]
    s32 cls;
    s32 veh;
    s32 unk10;
} RHDR;

typedef struct {
    f32 time;
    f32 stickX;
    f32 stickY;
    u32 buttons;
} RPKT;

typedef struct DemoRecording {
    struct DemoRecording* rec;
    DemoRecordingEntry entries[0x708];
} DemoRecording;

// .data
STATIC_DATA s32 sDemoMode = 0;

// .bss
STATIC_DATA DemoRecording* sDemoRecording;
STATIC_DATA f32 sDemoRecStartTime;
STATIC_DATA f32 sDemoRecEndTime;
STATIC_DATA s32 D_8036DA3C;
STATIC_DATA s32 D_8036DA40;
STATIC_DATA s32 sDemoPilotId;
STATIC_DATA s32 sDemoVehicleId;
STATIC_DATA RHDR sDemoRecHeader;

f32 demoRecGetStickX(DemoRecordingEntry* entry) {
    return (f32)(((f32)((entry->inputs & 0xFF000000) >> 24) / 127.0) - 1.0);
}

f32 demoRecGetStickY(DemoRecordingEntry* entry) {
    return (f32)(((f32)((entry->inputs & 0x00FF0000) >> 16) / 127.0) - 1.0);
}

u32 demoRecGetButtons(DemoRecordingEntry* entry) {
    return entry->inputs & 0x0000FFFF;
}

void demoPackInputs(DemoRecordingEntry* entry, f32 stickX, f32 stickY, u32 buttons) {
    entry->inputs = (u32)(((u32)((stickX + 1.0) * 127.0) << 0x18) | ((u32)((1.0 + stickY) * 127.0) << 0x10) | buttons);
}

s32 demoGetUserFileIdx(s32 pilot, s32 veh) {
    if (veh < 0) {
        switch (pilot) {
        case PILOT_LARK:
            return 0x55;
        case PILOT_GOOSE:
            return 0x57;
        case PILOT_HAWK:
            return 0x59;
        case PILOT_KIWI:
            return 0x56;
        case PILOT_IBIS:
            return 0x58;
        case PILOT_ROBIN:
            return 0x5A;
        default:
            return 0x55;
        }
    }

    switch (veh) {
    case VEHICLE_HANG_GLIDER:
        switch (pilot) {
        case PILOT_LARK:
            return 0x5B;
        case PILOT_GOOSE:
            return 0x5D;
        case PILOT_HAWK:
            return 0x5F;
        case PILOT_KIWI:
            return 0x5C;
        case PILOT_IBIS:
            return 0x5E;
        case PILOT_ROBIN:
            return 0x60;
        default:
            return 0x5B;
        }
    case VEHICLE_ROCKET_BELT:
        switch (pilot) {
        case PILOT_LARK:
            return 0x61;
        case PILOT_GOOSE:
            return 0x63;
        case PILOT_HAWK:
            return 0x65;
        case PILOT_KIWI:
            return 0x62;
        case PILOT_IBIS:
            return 0x64;
        case PILOT_ROBIN:
            return 0x66;
        default:
            return 0x61;
        }
    default:
        switch (pilot) {
        case PILOT_LARK:
            return 0x67;
        case PILOT_GOOSE:
            return 0x69;
        case PILOT_HAWK:
            return 0x6B;
        case PILOT_KIWI:
            return 0x68;
        case PILOT_IBIS:
            return 0x6A;
        case PILOT_ROBIN:
            return 0x6C;
        default:
            return 0x67;
        }
    }
}

s32 demoGetClassId(void) {
    return sDemoRecHeader.cls;
}

s32 demoGetVehicleId(void) {
    return sDemoRecHeader.veh;
}

s32 demoGet_80322B78(void) {
    return sDemoRecHeader.unk10;
}

s32 demoGetTestId(void) {
    return sDemoRecHeader.test;
}

void demoAllocate(void) {
    DemoRecording* rec;

    if ((sDemoRecording == NULL) || (sDemoRecording != sDemoRecording->rec)) {
        rec = (DemoRecording*)_uvMemAllocAlign8(sizeof(DemoRecording));
        sDemoRecording = rec;
        sDemoRecording->rec = sDemoRecording;
    }
}

void demoInit(void) {
    DemoRecordingEntry* entry;
    s32 temp_s4;
    s32 idx;
    s32 done;
    u32 size;
    u32 tag;
    void* data;
    RPKT pkt;

    idx = 0;
    done = 0;
    demoAllocate();
    temp_s4 = uvFileReadHeader((s32)uvUserFileRead(demoGetUserFileIdx(sDemoPilotId, sDemoVehicleId), 2));
    uvMemSet(sDemoRecording->entries, 0, sizeof(sDemoRecording->entries));
    uvMemSet(&sDemoRecHeader, 0, sizeof(sDemoRecHeader));

    do {
        tag = uvFileReadBlock(temp_s4, &size, &data, 1);
        switch (tag) {
        case 'RPKT': // 0x52504B54
            _uvMediaCopy(&pkt, data, size);
            sDemoRecording->entries[idx].time = pkt.time;
            entry = &sDemoRecording->entries[idx];
            demoPackInputs(entry, pkt.stickX, pkt.stickY, pkt.buttons);
            idx++;
            break;
        case 'RHDR': // 0x52484452
            _uvMediaCopy(&sDemoRecHeader, data, size);
            break;
        case 0:
            if (tag && tag) { } // fakematch
            done = 1;
            break;
        }
    } while ((!done) && (idx < ARRAY_COUNT(sDemoRecording->entries)));
    uvFile_80223F30(temp_s4);
}

void demoLoad(s32 pilot, s32 veh) {
    s32 userFileIdx;
    sDemoPilotId = pilot;
    sDemoVehicleId = veh;
    demoInit();
    userFileIdx = demoGetUserFileIdx(sDemoPilotId, sDemoVehicleId);
    demoAttInit(userFileIdx);
}

void demoSetRecMode(s32 mode) {
    switch (mode) {
    case 1:
        _uvDebugPrintf("Beginning input recording\n");
        sDemoRecStartTime = D_8034F850;
        sDemoRecEndTime = D_8034F850 + 60.0f;
        sDemoMode = mode;
        break;
    case 2:
        D_8036DA3C = 0;
        D_8036DA40 = 0;
        sDemoRecStartTime = D_8034F850;
        sDemoRecEndTime = D_8034F850 + 60.0f;
        sDemoMode = mode;
        break;
    case 0:
        if (sDemoMode == 1) {
            _uvDebugPrintf("End of recording\n");
        }
        sDemoMode = mode;
        break;
    }
    sDemoMode = mode;
    if (sDemoMode != 0) {
        uvRandSeed(0xABCD2356);
    }
}

s32 demoGetRecMode(void) {
    return sDemoMode;
}

void demoLogInput(void) {
    if (sDemoMode == 1) {
        _uvDebugPrintf("%.10f(%.10f, %.10f)0x%x\n", D_8034F850 - sDemoRecStartTime, uvControllerGetStick(0, 0), uvControllerGetStick(0, 1),
                       uvControllerGetButton(0));
    }
    if (sDemoRecEndTime <= D_8034F850) {
        demoSetRecMode(0);
    }
}

s32 demoGetEntryIndex(f32 curTime) {
    f32 temp_fv0;
    s32 var_v1;

    var_v1 = 0;
    temp_fv0 = curTime - sDemoRecStartTime;

    for (var_v1 = 0; var_v1 < ARRAY_COUNT(sDemoRecording->entries); var_v1++) {
        if (temp_fv0 <= sDemoRecording->entries[var_v1].time) {
            return var_v1;
        }
    }
    return ARRAY_COUNT(sDemoRecording->entries) - 1;
}

s32 demo_80323020(void) {
    s32 idx;
    if (sDemoMode == 2) {
        if ((sDemoRecording == NULL) || (sDemoRecording != sDemoRecording->rec)) {
            demoInit();
        }
        D_8036DA3C = D_8036DA40;
        idx = demoGetEntryIndex(D_8034F850);
        D_8036DA40 = demoRecGetButtons(&sDemoRecording->entries[idx]);
    }
    return uvIOUpdate();
}

f32 demoGetInputs(s32 contIdx, s32 axis) {
    s32 idx;
    if (sDemoMode == 2) {
        if ((sDemoRecording == NULL) || (sDemoRecording != sDemoRecording->rec)) {
            demoInit();
        }
        if (axis == INPUT_AXIS_X) {
            idx = demoGetEntryIndex(D_8034F850);
            return demoRecGetStickX(&sDemoRecording->entries[idx]);
        } else {
            idx = demoGetEntryIndex(D_8034F850);
            return demoRecGetStickY(&sDemoRecording->entries[idx]);
        }
    } else {
        return uvControllerGetStick(contIdx, axis);
    }
}

s32 demoGetButtons(s32 contIdx) {
    if (sDemoMode == 2) {
        return D_8036DA40;
    }
    return uvControllerGetButton(contIdx);
}

s32 demoButtonCheck(s32 contIdx, s32 buttonMask) {
    if (sDemoMode == 2) {
        return buttonMask & D_8036DA40;
    }
    return uvControllerButtonCheck(contIdx, buttonMask);
}

int demoButtonPress(s32 contIdx, s32 buttonMask) {
    if (sDemoMode == 2) {
        return demoButtonCheck(contIdx, buttonMask) == 0 && (buttonMask & D_8036DA3C) != 0;
    }
    return uvControllerButtonPress(contIdx, buttonMask);
}

int demoButtonRelease(s32 contIdx, s32 buttonMask) {
    if (sDemoMode == 2) {
        return demoButtonCheck(contIdx, buttonMask) == 0 && (buttonMask & D_8036DA3C) != 0;
    }
    return uvControllerButtonRelease(contIdx, buttonMask);
}

f32 demoRandF(void) {
    f32 rng;
    if (sDemoMode != 0) {
        rng = uvRandF_LCG();
    } else {
        rng = uvRandF_RANLUX();
    }
    return rng;
}
