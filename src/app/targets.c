#include "common.h"
#include <uv_dobj.h>
#include <uv_fx.h>
#include <uv_model.h>
#include <uv_texture.h>
#include "code_9A960.h"
#include "code_B2900.h"
#include "hud.h"
#include "snd.h"
#include "targets.h"
#include "task.h"
#include "text_data.h"

STATIC_DATA f32 D_803506A0[] = {
    0.0f, 0.0f, 0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f,  5.0f,  0.05f, 0.0f, 0.0f,  0.975f, 5.5f,  0.06f, 0.04f, 0.03f, 0.88f, 6.0f,
    0.0f, 0.0f, 0.0f, 0.0f,  17.0f, 0.0f, 0.0f, 0.0f, 0.0f,  18.0f, 0.0f,  0.0f, 0.01f, 0.15f,  18.5f, 0.0f,  0.01f, 0.03f, 0.25f, 19.0f,
    0.0f, 0.0f, 0.0f, 0.65f, 19.5f, 0.0f, 0.0f, 0.0f, 0.96f, 20.0f, 0.0f,  0.0f, 0.0f,  0.99f,  24.0f, 0.0f,  0.0f,  0.0f,  1.0f,
};
TaskObjects* D_8035078C = NULL;

TaskTARG* sRefTARG;
u8 sMissileTargetCount;
MissileTarget sMissileTargets[40];

void targetsInit(void) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sMissileTargets); i++) {
        sMissileTargets[i].objId = 0xFFFF;
        sMissileTargets[i].unk46 = 0;
        sMissileTargets[i].unk47 = 0;
        sMissileTargets[i].unk48 = 0xFF;
    }
}

void targetsLoad(void) {
    MissileTarget* var_s1;
    s32 i;
    TaskTARG* missileTarget;

    if (D_80362690->unkA0 == 0) {
        return;
    }
    sMissileTargetCount = taskGetTARG(&sRefTARG);
    if (sMissileTargetCount > ARRAY_COUNT(sMissileTargets)) {
        _uvDebugPrintf("targets : too many missile targets defined in level [%d]\n", sMissileTargetCount);
        sMissileTargetCount = 0;
        return;
    }

    if (sMissileTargetCount == 0) {
        return;
    }

    uvLevelAppend(0x12);
    for (i = 0; i < sMissileTargetCount; i++) {
        var_s1 = &sMissileTargets[i];
        missileTarget = &sRefTARG[i];
        if (var_s1->unk46 == 0) {
            if (var_s1->unk47 == 0) {
                var_s1->unk44 = missileTarget->targetType;
                var_s1->unk45 = missileTarget->unk19;
                // multiplies with almost DEG_TO_RAD(1)
                func_80313640(missileTarget->pos.x, missileTarget->pos.y, missileTarget->pos.z, missileTarget->rot.x * 0.0174533f,
                              missileTarget->rot.y * 0.0174533f, missileTarget->rot.z * 0.0174533f, &var_s1->unk4);
                var_s1->unk47 = 1;
            }
            var_s1->objId = uvDobjAllocIdx();
            var_s1->unk48 = hudAddWaypoint(missileTarget->pos.x, missileTarget->pos.y, missileTarget->pos.z);
            switch (missileTarget->targetType) {
            case 0:
                uvDobjModel(var_s1->objId, MODEL_MISSILE_TARGET_BALL);
                break;
            case 1:
                uvDobjModel(var_s1->objId, MODEL_MISSILE_TARGET_DISC);
                break;
            case 2:
                uvDobjModel(var_s1->objId, MODEL_MISSILE_TARGET_BILLBOARD);
                break;
            default:
                _uvDebugPrintf("targets : unknwwon target type [%d]\n", missileTarget->targetType);
                break;
            }
            uvDobjPosm(var_s1->objId, 0, &var_s1->unk4);
        }
    }
}

void targetsFrameUpdate(Mtx4F* arg0) {
    Mtx4F sp50;
    MissileTarget* var_s0;
    s32 i;
    s32 var_s4;

    var_s4 = 1;
    if ((D_80362690->unkA0 == 0) || ((s32)sMissileTargetCount <= 0)) {
        return;
    }

    *D_803506A0 += 3.1415923f * D_8034F854;
    if (*D_803506A0 > 6.2831855f) {
        *D_803506A0 -= 6.2831855f;
    }

    for (i = 0; i < sMissileTargetCount; i++) {
        var_s0 = &sMissileTargets[i];
        if ((var_s0->unk44 == 0) && (var_s0->unk46 == 0) && (var_s0->objId != 0xFFFF)) {
            if (var_s4 != 0) {
                var_s4 = 0;
                uvMat4SetIdentity(&sp50);
                uvMat4Copy(&sp50, &var_s0->unk4);
                uvMat4RotateAxis(&sp50, *D_803506A0, 'z');
            }
            sp50.m[3][0] = var_s0->unk4.m[3][0];
            sp50.m[3][1] = var_s0->unk4.m[3][1];
            sp50.m[3][2] = var_s0->unk4.m[3][2];
            uvDobjPosm(var_s0->objId, 0, &sp50);
        }
    }
}

void targetsDeinit(void) {
    s32 i;

    for (i = 0; i < sMissileTargetCount; i++) {
        if (sMissileTargets[i].objId != 0xFFFF) {
            uvDobjModel(sMissileTargets[i].objId, MODEL_WORLD);
            sMissileTargets[i].objId = 0xFFFF;
        }
        if (sMissileTargets[i].unk48 != 0xFF) {
            hud_8031A8E0(sMissileTargets[i].unk48);
            sMissileTargets[i].unk48 = 0xFF;
        }
    }
}

f32 targets_8034473C(s32 arg0, s32 arg1) {
    MissileTarget* var_s0;
    s32 i;
    s32 sp4C;
    s32 remaining;

    for (i = 0; i < sMissileTargetCount; i++) {
        var_s0 = &sMissileTargets[i];
        if (arg0 != var_s0->objId) {
            continue;
        }

        if (arg1 != 0 && var_s0->unk44 != 0) {
            return 1.0f;
        }

        if (arg1 != 0) {
            sndPlaySfx(SFX_UNK_056);
            hudText_8031D8E0(TEXT_B_HIT_B, 1.5f, 8.0f);
        } else {
            hudText_8031D8E0(TEXT_B_HIT_M, 1.5f, 8.0f);
        }

        func_8032C080(&remaining);
        if (remaining >= 2) {
#if defined(VERSION_JP)
            textFmtIntAt(textGetDataByIdx(TEXT_LEFT_CNT), remaining - 1, 2, 2);
#else // VERSION_US
            textFmtIntAt(textGetDataByIdx(TEXT_LEFT_CNT), remaining - 1, 2, 0);
#endif
            hudWarningText(TEXT_LEFT_CNT, 1.5f, 8.0f);
        }

        var_s0->unk46 = 1;
        sp4C = uvFx_8021EFF0(8);
        if (sp4C != 0xFF) {
            uvModelGet(sp4C, 8);
            uvFxProps(sp4C, FX_PROP_10(var_s0->unk4.m[3][0], var_s0->unk4.m[3][1], var_s0->unk4.m[3][2]), FX_PROP_END);
        }

        uvDobjModel(var_s0->objId, MODEL_WORLD);
        var_s0->objId = 0xFFFF;
        if (var_s0->unk48 != 0xFF) {
            hud_8031A8E0(var_s0->unk48);
            var_s0->unk48 = 0xFF;
        }
        return 0.0f;
    }

    return 1.0f;
}

u8 targets_803448F4(void) {
    u8 ret;
    s32 i;

    ret = 0;
    for (i = 0; i < sMissileTargetCount; i++) {
        ret += sMissileTargets[i].unk46;
    }
    return ret;
}

s16 targets_80344948(void) {
    s32 i;
    s16 var_v1;

    var_v1 = 0;
    for (i = 0; i < sMissileTargetCount; i++) {
        if (sMissileTargets[i].unk46 != 0) {
            var_v1 += sMissileTargets[i].unk45;
        }
    }

    return var_v1;
}
