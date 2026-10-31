#include "common.h"
#include "game.h"
#include "kernel/code_1050.h"
#include "snd.h"
#include <uv_audio.h>
#include <uv_math.h>
#include <uv_matrix.h>

Mtx4F D_80373E20;
Mtx4F D_80373E60;
Mtx4F D_80373EA0;
f32 D_80373EE0;
EventCallbackInfo D_80373EE8;

u16 gCurrentMusicId = 0xFF;
f32 D_803505B4 = 0.0f;
f32 D_803505B8 = 1.0f;
f32 D_803505BC = 0.0f;
u8 gMusicPlayingFlag = 0;
f32 D_803505C4[VEHICLE_COUNT][2] = {
    {  0.9f, 1.0f },
    {  0.8f, 1.0f },
    { 0.65f, 0.9f },
    {  1.0f, 0.5f },
    {  1.0f, 1.0f },
    {  1.0f, 1.0f },
    {  0.7f, 1.0f }
};
f32 D_803505FC[] = { 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

// forward declarations
void sndEvent(s32, void*, s32);

void sndInit(void) {
    uvMat4SetIdentity(&D_80373E20);
    uvMat4SetIdentity(&D_80373E60);
    uvMat4SetIdentity(&D_80373EA0);
    func_80200144(0, &D_80373E20);
    func_80200180(0, 2, 0.0f, 0);
    uvaManager_80204518(0);
    D_80373EE0 = 1.0f;
    D_80373EE8.cb = &sndEvent;
    D_80373EE8.arg = 0;
    uvEventMaxCb(D_80373EE8, 1, 36);
}

void func_8033F6F8(Mtx4F* arg0, Mtx4F* arg1) {
    uvMat4Copy(&D_80373E20, arg0);
    uvMat4Copy(&D_80373E60, arg1);
    func_80200144(0, &D_80373E20);
}

void sndSetMusic(u16 musicId) {
    gCurrentMusicId = musicId;
}

void sndPlaySfxVolPitchPan(u8 sfxId, f32 vol, f32 pitch, f32 pan) {
    u8 emitterId = uvEmitterLookup();
    if (emitterId == 0xFF) {
        _uvDebugPrintf("snd : out of local fx emitter devices\n");
        return;
    }
    uvEmitterFromModel(emitterId, sfxId);
    uvEmitterProps(emitterId, EMITTER_PROP_ATTR(EMITTER_ATTR_INIT | EMITTER_ATTR_ONESHOT), EMITTER_PROP_END);
    uvEmitterSetVol(emitterId, vol);
    uvEmitterSetPitch(emitterId, pitch);
    uvEmitterSetPan(emitterId, pan);
    uvEmitterTrigger(emitterId);
}

void sndPlaySfx(u8 sfxId) {
    u8 emitterId = uvEmitterLookup();
    if (emitterId == 0xFF) {
        _uvDebugPrintf("snd : out of local fx emitter devices\n");
        return;
    }
    uvEmitterFromModel(emitterId, sfxId);
    uvEmitterProps(emitterId, EMITTER_PROP_ATTR(EMITTER_ATTR_INIT | EMITTER_ATTR_ONESHOT), EMITTER_PROP_END);
    uvEmitterTrigger(emitterId);
}

u8 sndMakeDev(s32 sfxId) {
    u8 emitterId = uvEmitterLookup();
    if (emitterId == 0xFF) {
        _uvDebugPrintf("snd_makedev -- out of emitter devices\n");
        return 0xFF;
    }
    uvEmitterFromModel(emitterId, sfxId);
    uvEmitterSetVol(emitterId, 0.0f);
    uvEmitterTrigger(emitterId);
    return emitterId;
}

u8 func_8033F8CC(u8 emitterId) {
    if (emitterId != 0xFF) {
        uvEmitterFromModel(emitterId, 0xFF);
    }

    return 0xFF;
}

void func_8033F904(u8 emitterId, f32 pitch, f32 vol, f32 pan) {
    f32 vol_sq = uvSqrtF(vol);
    if (emitterId != 0xFF) {
        uvEmitterSetVol(emitterId, vol_sq);
        uvEmitterSetPitch(emitterId, pitch);
        uvEmitterSetPan(emitterId, pan);
    }
}

void sndSetMusicState(u8 state) {
    switch (state) {
    case MUS_STATE_PLAY_SEQ:
        if (gMusicPlayingFlag != 0) {
            uvaSeqStop();
        }
        D_803505BC = 0.0f;
        gMusicPlayingFlag = 1;
        uvaSeqNew(gCurrentMusicId);
        uvaSeqPlay();
        return;
    case MUS_STATE_STOP_SEQ:
        if (gMusicPlayingFlag != 0) {
            uvaSeqStop();
            D_803505BC = 0.0f;
            D_803505B4 = 0.0f;
            gMusicPlayingFlag = 0;
            return;
        }
        return;
    case MUS_STATE_PAUSE_EXIT:
        D_803505BC = 0.5f;
        if (gMusicPlayingFlag == 0) {
            D_803505B4 = 0 /*0.0f*/;
            uvaSeqNew(gCurrentMusicId);
            uvaSeqPlay();
            gMusicPlayingFlag = 1;
            return;
        }
        break;
    case MUS_STATE_PAUSE_MENU:
        D_803505BC = -0.5f;
        break;
    }
}

void func_8033FA88(f32 arg0) {
    if (gMusicPlayingFlag != 0) {
        func_80200180(0, 2, uvSqrtF(arg0), 0);
    }
}

void func_8033FAD4(f32 arg0) {
    D_80373EE0 = arg0;
    func_80200180(0, 1, uvSqrtF(arg0), 0);
}

void func_8033FB14(void) {
    uvSoundBegin();
    func_80200308(0);
    uvSoundEnd();
}

void sndEvent(s32 event, void* data, s32 eventData) {
    if (event != 1) {
        _uvAssertMsg("event == EVENT_FRM_END", "snd.c", 417);
    }
    if (data != NULL) {
        _uvAssertMsg("data == 0", "snd.c", 419);
    }
    if (eventData != 0) {
        _uvAssertMsg("eventData == 0", "snd.c", 421);
    }
    func_8033FB14();
}

void sndGetPilotScream(s32* soundId, f32* pitch) {
    Unk80362690_Unk0* temp = &D_80362690->unkC[D_80362690->unk9C];

    switch (temp->pilot) {
    case PILOT_LARK:
        *soundId = SFX_PILOT_LARK_SCREAM;
        *pitch = 1.1f;
        return;
    case PILOT_GOOSE:
        *soundId = SFX_PILOT_GOOSE_SCREAM;
        *pitch = 1.0f;
        return;
    case PILOT_HAWK:
        *soundId = SFX_PILOT_HAWK_SCREAM;
        *pitch = 0.6f;
        return;
    case PILOT_KIWI:
        *soundId = SFX_PILOT_KIWI_SCREAM;
        *pitch = 0.96f;
        return;
    case PILOT_IBIS:
        *soundId = SFX_PILOT_IBIS_SCREAM;
        *pitch = 1.0f;
        return;
    case PILOT_ROBIN:
        *soundId = SFX_PILOT_ROBIN_SCREAM;
        *pitch = 0.75f;
        return;
    default:
        _uvDebugPrintf("snd_getpilot:: Don't know scream for unknown pilot %d\n", temp->pilot);
        return;
    }
}

void func_8033FCD0(u16 arg0) {
    if (arg0 == 0xFF) {
        D_803505B8 = D_803505FC[0];
        D_803505B4 = D_803505FC[1];
    } else if (arg0 == 0xFE) {
        D_803505B8 = 0.0f;
        D_803505B4 = 1.0f;
    } else {
        D_803505B8 = D_803505C4[arg0][0];
        D_803505B4 = D_803505C4[arg0][1];
    }

    func_8033FAD4(D_803505B8);
    func_8033FA88(D_803505B4);
}

void func_8033FD94(u16 arg0, f32 arg1, f32 arg2) {
    s32 i;

    if (arg0 == 0xFF) {
        for (i = 0; i < ARRAY_COUNT(D_803505C4); i++) {
            D_803505C4[i][0] = arg1;
            D_803505C4[i][1] = arg2;
        }
        D_803505FC[0] = arg1;
        D_803505FC[1] = arg2;
    } else {
        D_803505C4[arg0][0] = arg1;
        D_803505C4[arg0][1] = arg2;
    }
}

