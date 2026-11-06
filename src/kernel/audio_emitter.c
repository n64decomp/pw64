#include <macros.h>
#include <uv_audio.h>
#include <uv_math.h>
#include <libc/stdarg.h>

#define AUD_NSFX_VOICES 16
#define AUD_NEMITTERS 0xFF

u8 sEmitterPlaylistObjects[AUD_NSFX_VOICES];
u8 sEmitterVoiceObjects[AUD_NSFX_VOICES];
s32 sNumPlaylistEmitters;
uvaEmitter gSndEmitterTable[256];

void uvEmitterPrintf(const char* fmt, ...) {
}

void uvEmitterInitTable(void) {
    s32 i;
    for (i = 0; i < 0xFF; i++) {
        uvEmitterInit(&gSndEmitterTable[i]);
    }
    uvEmitterInit(&gSndEmitterTable[255]);
}

void uvEmitterInit(uvaEmitter* emitter) {
    Mtx4F temp;

    uvMat4SetIdentity(&temp);
    emitter->playVoice = 0x11;
    emitter->playState = 0;
    emitter->playVolume = 0x7FFF;
    emitter->playPan = 0x40;
    emitter->playMix = 0;
    emitter->playTimeout = 0;
    emitter->playPitch = 1.0f;

    uvMat4Copy(&emitter->m1, &temp);
    emitter->sound = 0xFF;
    emitter->priority = 0;
    emitter->attr = 0;
    emitter->state = EMITTER_STATE_RELEASED;

    emitter->unk40.x = 0.0f;
    emitter->unk40.y = 0.0f;
    emitter->unk40.z = 0.0f;

    emitter->unk4C.x = 0.0f;
    emitter->unk4C.y = 0.0f;
    emitter->unk4C.z = 0.0f;

    emitter->vol = 1.0f;
    emitter->pitch = 1.0f;
    emitter->pan = 0.0f;
    emitter->mix = 0.0f;

    emitter->unk80 = 0.0f;
    emitter->unk84 = 0.0f;
    emitter->near = 0.0f;
    emitter->far = 1000.0f;
}

u8 uvEmitterLookup(void) {
    u8 emitterId;

    uvEmitterPrintf("looking for emitter\n");
    for (emitterId = 0; emitterId < AUD_NEMITTERS; emitterId++) {
        if (gSndEmitterTable[emitterId].sound == 0xFF) {
            uvEmitterPrintf("Returning emitter %d\n", emitterId);
            return emitterId;
        }
    }
    _uvDebugPrintf("no free emitters\n");
    return 0xFF;
}

void uvEmitterFromModel(u8 emitterId, u8 modelId) {
    uvEmitterPrintf("assigning model %d to emitter %d\n", modelId, emitterId);
    if (emitterId < AUD_NEMITTERS) {
        if (gSndEmitterTable[emitterId].playState != 0x11) {
            _uvaStopVoice(gSndEmitterTable[emitterId].playVoice);
            gSndEmitterTable[emitterId].playState = 0;
            gSndEmitterTable[emitterId].playVoice = 0x11U;
            gSndEmitterTable[emitterId].playTimeout = 0;
        }
        uvEmitterInit(&gSndEmitterTable[emitterId]);
        gSndEmitterTable[emitterId].sound = modelId;
    }
}

void uvEmitterSetMatrix(u8 emitterId, Mtx4F* src) {
    if (emitterId < AUD_NEMITTERS) {
        uvMat4Copy(&gSndEmitterTable[emitterId].m1, src);
    }
}

void uvEmitterGetMatrix(u8 emitterId, Mtx4F* dst) {
    if (emitterId < AUD_NEMITTERS) {
        uvMat4Copy(dst, &gSndEmitterTable[emitterId].m1);
    }
}

void uvEmitter_80201494(u8 emitterId, Vec3F arg1, Vec3F arg4) {
    Vec3F sp44;
    f32 temp_fv0;
    uvaEmitter* emitter;

    if (emitterId >= AUD_NEMITTERS) {
        return;
    }

    emitter = &gSndEmitterTable[emitterId];
    uvVec3Copy(&emitter->unk40, &arg1);
    uvVec3Copy(&emitter->unk4C, &arg4);
    emitter->unk64.x = (emitter->unk4C.x + emitter->unk40.x) * 0.5f;
    emitter->unk64.y = (emitter->unk4C.y + emitter->unk40.y) * 0.5f;
    emitter->unk64.z = (emitter->unk4C.z + emitter->unk40.z) * 0.5f;
    sp44.z = emitter->unk4C.x - emitter->unk40.x;
    sp44.y = emitter->unk4C.y - emitter->unk40.y;
    sp44.x = emitter->unk4C.z - emitter->unk40.z;
    temp_fv0 = uvSqrtF(SQ(sp44.z) + SQ(sp44.y) + SQ(sp44.x));
    if (temp_fv0 != 0.0f) {
        emitter->unk58.x = sp44.z / temp_fv0;
        emitter->unk58.y = sp44.y / temp_fv0;
        emitter->unk58.z = sp44.x / temp_fv0;
    }
}

void uvEmitterSetVol(u8 emitterId, f32 vol) {
    if (emitterId < AUD_NEMITTERS) {
        gSndEmitterTable[emitterId].vol = vol;
    }
}

f32 uvEmitterGetVol(u8 emitterId) {
    if (emitterId >= AUD_NEMITTERS) {
        return 0.0f;
    }
    return gSndEmitterTable[emitterId].vol;
}

void uvEmitterSetPan(u8 emitterId, f32 pan) {
    if (emitterId < AUD_NEMITTERS) {
        gSndEmitterTable[emitterId].pan = pan;
    }
}

void uvEmitterSetPitch(u8 emitterId, f32 pitch) {
    if (emitterId < AUD_NEMITTERS) {
        gSndEmitterTable[emitterId].pitch = pitch;
    }
}

void uvEmitterSetPri(u8 emitterId, s32 pri) {
    if (emitterId < AUD_NEMITTERS) {
        gSndEmitterTable[emitterId].priority = pri;
    }
}

void uvEmitterProps(u8 emitterId, ...) {
    f32 argValue;
    uvaEmitter* obj;
    s32 propertyType;
    va_list args;

    if (emitterId >= AUD_NEMITTERS || emitterId == 0xFF) {
        return;
    }

    obj = &gSndEmitterTable[emitterId];

    // passing an object that undergoes default argument promotion to 'va_start'
    // has undefined behavior (e.g. u8, u16, f32)
#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvarargs"
#endif
    va_start(args, emitterId);
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif

    while (TRUE) {
        propertyType = va_arg(args, s32);
        switch (propertyType) {
        case EMITTER_PROPID_END:
            return;
        case EMITTER_PROPID_NEAR:
            argValue = va_arg(args, f64);
            obj->near = SQ(argValue);
            break;
        case EMITTER_PROPID_FAR:
            argValue = va_arg(args, f64);
            obj->far = SQ(argValue);
            break;
        case EMITTER_PROPID_4:
            obj->unk84 = va_arg(args, f64);
            break;
        case EMITTER_PROPID_3:
            obj->unk80 = va_arg(args, f64);
            break;
        case EMITTER_PROPID_ATTR:
            obj->attr = va_arg(args, s32);
            break;
        default:
            uvEmitterPrintf("Unknown property type 0x%x\n", propertyType);
            return;
        }
    }
}

void uvEmitterTrigger(u8 emitterId) {
    if (emitterId < AUD_NEMITTERS) {
        uvEmitterPrintf("\nemitter %d triggered\n", emitterId);
        gSndEmitterTable[emitterId].state = EMITTER_STATE_TRIGGERED;
    }
}

void uvEmitterRelease(u8 emitterId) {
    if (emitterId < AUD_NEMITTERS) {
        uvEmitterPrintf("\nreleasing trigger on %d\n", emitterId);

        if (gSndEmitterTable[emitterId].playVoice != 0x11) {
            _uvaStopVoice(gSndEmitterTable[emitterId].playVoice);
        }
        gSndEmitterTable[emitterId].state = EMITTER_STATE_RELEASED;
        gSndEmitterTable[emitterId].playVoice = 0x11;
        gSndEmitterTable[emitterId].playState = 0;
        gSndEmitterTable[emitterId].playTimeout = 0;
        if (gSndEmitterTable[emitterId].attr & EMITTER_ATTR_INIT) {
            uvEmitterInit(&gSndEmitterTable[emitterId]);
        }
    }
}

void uvEmitterStatus(u8 arg0) {
    u8 i;
    u8 emitterCount;

    for (i = 0, emitterCount = 0; i < AUD_NEMITTERS; i++) {
        if (gSndEmitterTable[i].sound != 0xFF) {
            if (arg0 != 0) {
                gSndEmitterTable[i].attr |= EMITTER_ATTR_INIT;
                uvEmitterRelease(i);
            }
            emitterCount++;
        }
    }

    if (emitterCount > 0) {
        _uvDebugPrintf("@@@  %d Emitters %s\n", emitterCount, (arg0 != 0) ? "released" : "still in use");
    }

    if (sNumPlaylistEmitters > 0) {
        _uvDebugPrintf("###  %d emitters on active playlist\n", sNumPlaylistEmitters);
    }
}

void uvSoundBegin(void) {
    s32 i;

    uvEmitterPrintf("\nsoundBegin\n");

    for (i = 0; i < AUD_NSFX_VOICES; i++) {
        sEmitterPlaylistObjects[i] = 255;
        sEmitterVoiceObjects[i] = 255;
    }

    sNumPlaylistEmitters = 0;
}

void uvSoundEnd(void) {
    uvEmitterPrintf("\nsoundEnd\n");
    _uvaPlay();
}

void uvEmitterFlush(u16 arg0) {
    s32 i;

    for (i = 0; i < AUD_NEMITTERS; i++) {
        if ((gSndEmitterTable[i].playState == 1) || (gSndEmitterTable[i].playState == 2) || (gSndEmitterTable[i].playState == 8) ||
            (gSndEmitterTable[i].state == EMITTER_STATE_TRIGGERED)) {
            if (gSndEmitterTable[i].playState == 2) {
                gSndEmitterTable[i].playTimeout++;
            }
            uvEmitter_80201D08(i, arg0);
            if (((gSndEmitterTable[i].playVolume + gSndEmitterTable[i].priority) == 0) ||
                ((gSndEmitterTable[i].playState == 2) && (gSndEmitterTable[i].playTimeout >= 5))) {
                uvEmitterPrintf("Flushing object %d due to timeout or zero priority\n", i);
                if (gSndEmitterTable[i].attr & EMITTER_ATTR_ONESHOT) {
                    uvEmitterRelease(i);
                } else {
                    uvEmitterRelease(i);
                    uvEmitterTrigger(i);
                }
            } else {
                _uvaUpdatePlayList(i);
            }
        }
    }
}

// variable 'sp84' is used uninitialized whenever 'if' condition is true. see @bug below
#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsometimes-uninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
void uvEmitter_80201D08(u8 emitterId, u16 arg1) {
    s32 pan;
    f32 vol;
    Vec3F spAC;
    Vec3F spA0;
    Vec3F sp94;
    Vec3F sp88;
    f32 sp84;
    f32 temp_fv0;
    f32 var_fa0;
    Mtx4F sp3C;
    f32 sp38;
    f32 sp34;
    f32 sp30;
    uvaEmitter* emitter;
    s32 attr;

    sp38 = D_80250E80[arg1].unk40;
    sp34 = D_80250E80[arg1].unk48;
    sp30 = D_80250E80[arg1].unk4C;
    emitter = &gSndEmitterTable[emitterId];
    attr = emitter->attr;
    uvMat4Copy(&sp3C, &D_80250E80[arg1].mat);
    if (attr & EMITTER_ATTR_8) {
        spA0.x = sp3C.m[0][0];
        spA0.y = sp3C.m[0][1];
        spA0.z = sp3C.m[0][2];
        spAC.x = sp3C.m[3][0];
        spAC.y = sp3C.m[3][1];
        spAC.z = sp3C.m[3][2];
        if (attr & EMITTER_ATTR_2) {
            sp94.x = spAC.x - emitter->unk40.x;
            sp94.y = spAC.y - emitter->unk40.y;
            sp94.z = spAC.z - emitter->unk40.z;
            if (uvVec3Dot(&sp94, &emitter->unk58) < 0.0f) {
                sp84 = SQ(sp94.x) + SQ(sp94.y) + SQ(sp94.z);
            } else {
                sp88.x = -emitter->unk58.x;
                sp88.y = -emitter->unk58.y;
                sp88.z = -emitter->unk58.z;
                sp94.x = spAC.x - emitter->unk4C.x;
                sp94.y = spAC.y - emitter->unk4C.y;
                sp94.z = spAC.z - emitter->unk4C.z;
                temp_fv0 = uvVec3Dot(&sp94, &sp88);
                if (temp_fv0 < 0.0f) {
                    sp84 = SQ(sp94.x) + SQ(sp94.y) + SQ(sp94.z);
                } else {
                    sp94.x = spAC.x - ((temp_fv0 * sp88.x) + emitter->unk4C.x);
                    sp94.y = spAC.y - ((temp_fv0 * sp88.y) + emitter->unk4C.y);
                    sp94.z = spAC.z - ((temp_fv0 * sp88.z) + emitter->unk4C.z);
                    sp84 = SQ(sp94.x) + SQ(sp94.y) + SQ(sp94.z);
                }
            }
        } else if (attr & EMITTER_ATTR_1) {
            //! @bug sp84 is uninitialised in this code path
        } else {
            sp94.x = emitter->m1.m[3][0] - sp3C.m[3][0];
            sp94.y = emitter->m1.m[3][1] - sp3C.m[3][1];
            sp94.z = emitter->m1.m[3][2] - sp3C.m[3][2];
            sp84 = SQ(sp94.x) + SQ(sp94.y) + SQ(sp94.z);
        }

        if (emitter->far < sp84) {
            emitter->playPan = 0x40;
            emitter->playVolume = 0;
            emitter->playPitch = emitter->pitch;
            return;
        }

        if (sp84 <= emitter->near) {
            vol = emitter->vol;
        } else {
            vol = (emitter->vol * (emitter->far - sp84)) / (emitter->far - emitter->near);
        }
        if ((sp30 != 1.0f) && (sp84 != 0.0f)) {
            var_fa0 = uvVec3ScalarProj(&sp94, &spA0);

            if (sp84 < emitter->near) {
                var_fa0 *= sp84 / emitter->near;
            }
            if (attr & EMITTER_ATTR_2) {
                var_fa0 *= -1.0f;
            }
            pan = (s32)((1.0f + var_fa0) * 64.0f);
            if (pan == 0x80) {
                pan = 0x7F;
            }
        } else {
            pan = 0x40;
        }
    } else {
        vol = emitter->vol;
        if (sp30 != 1) {
            pan = (s32)((emitter->pan + 1.0f) * 64.0f);
            if (pan == 0x80) {
                pan = 0x7F;
            }
        } else {
            pan = 0x40;
        }
    }
    if (attr & EMITTER_ATTR_4) {
        emitter->playMix = (s32)(sp34 * 127.0f);
    } else {
        emitter->playMix = (s32)(emitter->mix * 127.0f);
    }
    emitter->playPan = pan;
    emitter->playPitch = emitter->pitch;
    emitter->playVolume = (s32)(vol * sp38 * 32767.0f);
}
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

void _uvaUpdatePlayList(u8 emitterId) {
    s32 i;
    s32 j;
    s32 priority;
    s32 newPriority;
    s32 insertedObj;

    insertedObj = FALSE;
    uvEmitterPrintf("\ninserting object %d in playlist\n", emitterId);

    if ((gSndEmitterTable[emitterId].attr & EMITTER_ATTR_ONESHOT) && gSndEmitterTable[emitterId].playState == 8) {
        uvEmitterPrintf("CHECKING if to delete oneshot object %d\n", emitterId);
        alSndpSetSound(gSndPlayer, gSndVoiceTable[gSndEmitterTable[emitterId].playVoice]);
        if ((alSndpGetState(gSndPlayer) == AL_STOPPED)) {
            _uvaStopVoice(gSndEmitterTable[emitterId].playVoice);
            gSndEmitterTable[emitterId].playVoice = 0x11;
            gSndEmitterTable[emitterId].playState = 0;
            if (gSndEmitterTable[emitterId].attr & EMITTER_ATTR_INIT) {
                uvEmitterPrintf("Object %d expunged due to finish\n", emitterId);
                uvEmitterInit(&gSndEmitterTable[emitterId]);
            }
            return;
        }
    }

    if (sNumPlaylistEmitters == 0) {
        uvEmitterPrintf("object %d first in list\n", emitterId);
        sEmitterPlaylistObjects[0] = emitterId;
        sNumPlaylistEmitters++;
        if (gSndEmitterTable[emitterId].playVoice != 0x11) {
            uvEmitterPrintf("object %d (1st in list) already has a voice\n", emitterId);
            sEmitterVoiceObjects[gSndEmitterTable[emitterId].playVoice] = emitterId;
        }
        uvEmitterPrintf("_uvaUpdatePlayList finished for object %d\n", emitterId);
        return;
    }
    priority = gSndEmitterTable[emitterId].priority + gSndEmitterTable[emitterId].playVolume;
    uvEmitterPrintf("priority = %d for object %d\n", priority, emitterId);

    for (i = 0; (i < sNumPlaylistEmitters) && !insertedObj; i++) {
        uvEmitterPrintf("i = %d\n", i);

        newPriority = gSndEmitterTable[sEmitterPlaylistObjects[i]].playVolume + gSndEmitterTable[sEmitterPlaylistObjects[i]].priority;
        uvEmitterPrintf("new Priority %d\n", newPriority);
        if (priority >= newPriority) {
            insertedObj = TRUE;
            uvEmitterPrintf("inserting in playlist\n");
            if (sNumPlaylistEmitters == AUD_NSFX_VOICES) {
                uvEmitterPrintf("flushing last item in playlist - %d\n", sEmitterPlaylistObjects[sNumPlaylistEmitters - 1]);

                if (gSndEmitterTable[sEmitterPlaylistObjects[sNumPlaylistEmitters - 1]].playVoice != 0x11) {
                    sEmitterVoiceObjects[gSndEmitterTable[sEmitterPlaylistObjects[sNumPlaylistEmitters - 1]].playVoice] = 0xFF;
                    uvEmitterPrintf("object %d had voice already\n", emitterId);
                }
                _uvaPlaylistRemove(sEmitterPlaylistObjects[sNumPlaylistEmitters - 1]);
            }

            for (j = sNumPlaylistEmitters; j > i; j--) {
                sEmitterPlaylistObjects[j] = sEmitterPlaylistObjects[j - 1];
            }
            sEmitterPlaylistObjects[i] = emitterId;
            if (gSndEmitterTable[emitterId].playVoice != 0x11) {
                sEmitterVoiceObjects[gSndEmitterTable[emitterId].playVoice] = emitterId;
                uvEmitterPrintf("object %d had voice already\n", emitterId);
            }
        }
    }

    if (insertedObj == FALSE && (i == AUD_NSFX_VOICES)) {
        uvEmitterPrintf("object %d flushed\n", emitterId);
        if (gSndEmitterTable[emitterId].playVoice != 0x11) {
            sEmitterVoiceObjects[gSndEmitterTable[emitterId].playVoice] = 0xFF;
        }
        _uvaPlaylistRemove(emitterId);
        return;
    }

    if (insertedObj == FALSE && (i == sNumPlaylistEmitters)) {
        uvEmitterPrintf("object %d last in list\n", emitterId);
        sEmitterPlaylistObjects[i] = emitterId;
        sNumPlaylistEmitters++;
        if (gSndEmitterTable[emitterId].playVoice != 0x11) {
            uvEmitterPrintf("object %d (last in list) already has a voice\n", emitterId);
            sEmitterVoiceObjects[gSndEmitterTable[emitterId].playVoice] = emitterId;
        }
        uvEmitterPrintf("_uvaUpdatePlayList finished for object %d\n", emitterId);
        return;
    }

    if (sNumPlaylistEmitters < AUD_NSFX_VOICES) {
        sNumPlaylistEmitters++;
    }

    uvEmitterPrintf("%d used voices\n", sNumPlaylistEmitters);
    uvEmitterPrintf("exiting _uvaUpdatePlayList\n");
}

void _uvaPlaylistRemove(u8 emitterId) {
    uvaEmitter* emitter = &gSndEmitterTable[emitterId];

    uvEmitterPrintf("\nRemoving object %d from playlist\n", emitterId);
    if (emitter->attr & EMITTER_ATTR_ONESHOT) {
        uvEmitterRelease(emitterId);
        return;
    }

    if (emitter->playVoice != 0x11) {
        _uvaStopVoice(emitter->playVoice);
        emitter->playVoice = 0x11;
        emitter->playState = 0;
        emitter->playTimeout = 0;
    }

    uvEmitterTrigger(emitterId);
}

void _uvaPlay(void) {
    s32 i;
    s32 j;

    uvEmitterPrintf("\n_uvaPlay %d voices busy\n", sNumPlaylistEmitters);

    for (i = 0; i < sNumPlaylistEmitters; i++) {
        uvEmitterPrintf("playing object %d\n", sEmitterPlaylistObjects[i]);
        _uvaStatus(sEmitterPlaylistObjects[i]);

        if (gSndEmitterTable[sEmitterPlaylistObjects[i]].playState == 0) {
            j = 0;
            while (sEmitterVoiceObjects[j] != 0xFF && j < (AUD_NSFX_VOICES - 1)) {
                j++;
            }

            uvEmitterPrintf("object %d getting a new voice: %d, j = %d\n", sEmitterPlaylistObjects[i], sEmitterVoiceObjects[j], j);

            sEmitterVoiceObjects[j] = sEmitterPlaylistObjects[i];
            gSndEmitterTable[sEmitterPlaylistObjects[i]].playVoice = j;
            _uvaStartVoice(sEmitterPlaylistObjects[i]);
        }

        if (gSndEmitterTable[sEmitterPlaylistObjects[i]].playState == 1) {
            _uvaStartVoice(sEmitterPlaylistObjects[i]);
        }

        if (gSndEmitterTable[sEmitterPlaylistObjects[i]].playState == 2) {
            uvEmitterPrintf("Checking if object %d is playing\n", sEmitterPlaylistObjects[i]);
            alSndpSetSound(gSndPlayer, gSndVoiceTable[gSndEmitterTable[sEmitterPlaylistObjects[i]].playVoice]);
            if (alSndpGetState(gSndPlayer) == AL_PLAYING) {
                uvEmitterPrintf("object %d is playing\n", sEmitterPlaylistObjects[i]);
                gSndEmitterTable[sEmitterPlaylistObjects[i]].playState = 8;
                gSndEmitterTable[sEmitterPlaylistObjects[i]].playTimeout = 0;
            }
        }

        if (gSndEmitterTable[sEmitterPlaylistObjects[i]].playState == 8) {
            _uvaUpdateVoice(sEmitterPlaylistObjects[i]);
        }
    }
}

void _uvaStartVoice(u8 emitterId) {
    ALInstrument* instrument;

    uvEmitterPrintf("\n_uvaStartVoice object %d -- voice %d\n", emitterId, gSndEmitterTable[emitterId].playVoice);
    gSndEmitterTable[emitterId].playState = 1;
    alSndpSetSound(gSndPlayer, gSndVoiceTable[gSndEmitterTable[emitterId].playVoice]);
    if (alSndpGetState(gSndPlayer) != AL_STOPPED) {
        alSndpStop(gSndPlayer);
        return;
    }
    alSndpDeallocate(gSndPlayer, gSndVoiceTable[gSndEmitterTable[emitterId].playVoice]);
    instrument = gAudioSXBank->instArray[gSndEmitterTable[emitterId].sound];
    gSndVoiceTable[gSndEmitterTable[emitterId].playVoice] = alSndpAllocate(gSndPlayer, instrument->soundArray[0]);
    alSndpSetSound(gSndPlayer, gSndVoiceTable[gSndEmitterTable[emitterId].playVoice]);
    if (gSndVoiceTable[gSndEmitterTable[emitterId].playVoice] >= 0) {
        _uvaUpdateVoice(emitterId);
        alSndpPlay(gSndPlayer);
        gSndEmitterTable[emitterId].playState = 2;
        gSndEmitterTable[emitterId].state = EMITTER_STATE_RELEASED;
    }
}

void _uvaUpdateVoice(u8 emitterId) {
    uvaEmitter* emitter = &gSndEmitterTable[emitterId];
    uvEmitterPrintf("\n  updating voice %d on object %d\n", emitter->playVoice, emitterId);

    alSndpSetSound(gSndPlayer, gSndVoiceTable[emitter->playVoice]);
    alSndpSetVol(gSndPlayer, emitter->playVolume);
    alSndpSetPitch(gSndPlayer, emitter->playPitch);
    alSndpSetFXMix(gSndPlayer, emitter->playMix);
    alSndpSetPan(gSndPlayer, emitter->playPan);
}

void _uvaStopVoice(u8 voiceId) {
    if (voiceId < AUD_NSFX_VOICES) {
        uvEmitterPrintf("stopping voice%d\n", voiceId);
        alSndpSetSound(gSndPlayer, gSndVoiceTable[voiceId]);
        if (alSndpGetState(gSndPlayer) == AL_PLAYING) {
            alSndpSetVol(gSndPlayer, 0);
        }
        alSndpStop(gSndPlayer);
    } else {
        uvEmitterPrintf("%d is not a valid voice\n", voiceId);
    }
}

void _uvaStatus(u8 emitterId) {
    uvaEmitter* emitter = &gSndEmitterTable[emitterId];
    uvEmitterPrintf("-----object %d------\n", emitterId);
    uvEmitterPrintf("playVoice  = %d\n", emitter->playVoice);
    uvEmitterPrintf("playState  = %d\n", emitter->playState);
    uvEmitterPrintf("playTimeout= %d\n", emitter->playTimeout);
    uvEmitterPrintf("playVolume = %d\n", emitter->playVolume);
    uvEmitterPrintf("playPitch  = %f\n", emitter->playPitch);
    uvEmitterPrintf("playPan    = %d\n", emitter->playPan);
    uvEmitterPrintf("near       = %f\n", emitter->near);
    uvEmitterPrintf("far        = %f\n", emitter->far);
    uvEmitterPrintf("sound      = %d\n", emitter->sound);
    uvEmitterPrintf("priority   = %d\n", emitter->priority);
    uvEmitterPrintf("state      = %d\n", emitter->state);
}

