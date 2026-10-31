#include "common.h"
#include <uv_chan.h>
#include <uv_dobj.h>
#include <uv_graphics.h>
#include <uv_math.h>
#include <uv_sobj.h>
#include "kernel/code_7150.h"
#include "kernel/code_8170.h"

extern s32 D_80248DD8;

void uvSobjsDraw(UnkStruct_80204D94* arg0, Mtx4F* arg1, u16 arg2, u16 arg3, UnkSobjDraw* arg4, u16 arg5, u8 arg6, s32 arg7) {
    UnkSobjDraw* var_s2;
    f32 temp_fs0;
    f32 temp_fs1;
    f32 temp_fs2;
    f32 temp_fv0;
    u32 lodId;
    u8 var_s1;
    ParsedUVMD* uvmd;
    ParsedUVMD** uvmdTable;
    f32 spB8;
    f32 spB4;
    f32 spB0;
    f32 spAC;
    f32 spA8;
    s32 i;

    spB0 = arg0->unk1D0 - arg1->m[3][0];
    spAC = arg0->unk1D4 - arg1->m[3][1];
    spA8 = arg0->unk1D8 - arg1->m[3][2];
    func_80214840(arg6, &spB0, &spAC);

    uvmdTable = gGfxUnkPtrs->models;
    for (i = 0; i < arg5; i++) {
        var_s2 = &arg4[i];

        if (var_s2->modelId == 0xFFFF) {
            continue;
        }

        if (!(var_s2->unk14 & arg2)) {
            continue;
        }
        uvmd = uvmdTable[var_s2->modelId];
        if (uvmd == NULL) {
            _uvDebugPrintf("uvSobjsDraw: model %d not in level\n");
            continue;
        }

        temp_fs1 = var_s2->unk8 - spB0;
        temp_fs2 = var_s2->unkC - spAC;
        temp_fs0 = var_s2->unk10 - spA8;
        if (!(var_s2->unk14 & arg3)) {
            spB8 = temp_fs1;
            spB4 = temp_fs2;
            func_80214840(arg6 * -1, &spB8, &spB4);
            if (func_80206F64(arg0->unk2E0, spB8, spB4, temp_fs0, uvmd->unk1C) == 0) {
                continue;
            }
        }
        temp_fv0 = uvSqrtF(SQ(temp_fs1) + SQ(temp_fs2) + SQ(temp_fs0));
        if (arg0->unk200 == 1.0f) {
            var_s1 = lodId = uvSobjGetLODIndex(uvmd, temp_fv0);
        } else {
            var_s1 = lodId = uvSobjGetLODIndex(uvmd, arg0->unk200 * temp_fv0);
        }
        if (lodId == 0xFF) {
            continue;
        }
        if (uvmd->attrs & UVMD_ATTR_TRANSPARENT) {
            _uvSortAdd(4, temp_fv0, var_s2, arg0, arg1->m[3][0], arg1->m[3][1], temp_fs1, temp_fs2, arg1, arg7, var_s1);
        } else if (uvmd->lodTable[lodId].billboard != FALSE) {
            uvSobj_8022CC28(var_s2, uvmd, var_s1, temp_fs1, temp_fs2);
        } else {
            uvSobj_8022C8D0(var_s2, uvmd, var_s1, arg1);
        }
    }
}

u8 uvSobjGetLODIndex(ParsedUVMD* uvmd, f32 dist) {
    s32 lodCount;
    s32 i;
    f32* lodRadius;

    lodRadius = uvmd->lodRadius;
    lodCount = uvmd->lodCount;
    if (lodRadius[lodCount - 1] <= dist) {
        return 0xFF;
    }

    for (i = lodCount; i > 0; i--) {
        if (lodRadius[i - 1] < dist) {
            return i;
        }
    }

    return 0;
}

void uvSobj_8022C8D0(UnkSobjDraw* arg0, ParsedUVMD* uvmd, u8 arg2, Mtx4F* arg3) {
    uvModelLOD* currLod;
    uvModelPart* currPart;
    s32 temp_v0;
    s32 i;
    s32 j;
    Mtx4F sp114;
    Mtx4F spD4;
    s32 pad;
    Mtx sp90;

    currLod = &uvmd->lodTable[arg2];

    for (i = 0; i < currLod->partCount; i++) {
        currPart = &currLod->partTable[i];
        uvGfx_802235A4(arg0->unk4[i], 1);
        if (currPart->lighting != FALSE) {
            uvMat4CopyL2F(&sp114, arg0->unk4[i]);
            sp114.m[3][0] += arg3->m[3][0];
            sp114.m[3][1] += arg3->m[3][1];
            sp114.m[3][2] += arg3->m[3][2];
            uvGfxLookAt(&sp114);
        }

        for (j = 0; j < currPart->stateCount; j++) {
            uvGfxStateDraw(&currPart->stateTable[j]);
        }
        if (i == currLod->partCount - 1) {
            break;
        }

        for (j = 0; j <= currLod->partTable[i].unk6 - currLod->partTable[i + 1].unk6; j++) {
            uvGfxMtxViewPop();
        }
    }
    temp_v0 = currLod->partTable[currLod->partCount - 1].unk6;
    for (i = 0; i <= temp_v0; i++) {
        uvGfxMtxViewPop();
    }
    if (D_80248DD8 != 0) {
        uvMat4CopyL2F(&spD4, arg0->unk4[0]);

        uvMat4Scale(&spD4, uvmd->unk20, uvmd->unk20, uvmd->unk20);
        uvMat4CopyF2L(&sp90, &spD4);
        uvGfx_802235A4(sp90, 1);
        func_80215E7C(uvmd);
        uvGfxMtxViewPop();
    }
}

void uvSobj_8022CC28(UnkSobjDraw* arg0, ParsedUVMD* uvmd, u8 arg2, f32 arg3, f32 arg4) {
    uvMtx* temp_s6;
    s32 temp_v1;
    s32 i;
    s32 j;
    Mtx sp88;
    f32 temp_fv0;
    uvModelPart* currPart;
    uvModelPart* temp_v0; // unused
    uvModelLOD* currLod;
    s32 pad;
    f32 sp70;

    sp70 = uvmd->unk20 / gGfxUnkPtrs->unk1608;
    currLod = &uvmd->lodTable[arg2];
    temp_s6 = (uvMtx*)arg0->unk4;
    sp88.m[0][0] = temp_s6->m[0][0];
    sp88.m[0][2] = temp_s6->m[0][2];
    sp88.m[2][0] = temp_s6->m[2][0];
    sp88.m[2][2] = temp_s6->m[2][2];
    temp_fv0 = uvSqrtF(SQ(arg3) + SQ(arg4));

    if (temp_fv0 != 0.0f) {
        arg3 /= temp_fv0;
        arg4 /= temp_fv0;
    }

    temp_s6->u.i[0][0] = (s32)(((-arg4 / sp70) * 65536.0f) + 0.5f) >> 0x10;
    temp_s6->u.i[0][1] = (s32)(((arg3 / sp70) * 65536.0f) + 0.5f) >> 0x10;
    temp_s6->u.f[0][0] = (s32)(((-arg4 / sp70) * 65536.0f) + 0.5f) & 0xFFFF;
    temp_s6->u.f[0][1] = (s32)(((arg3 / sp70) * 65536.0f) + 0.5f) & 0xFFFF;
    temp_s6->u.i[1][1] = (s32)(((-arg4 / sp70) * 65536.0f) + 0.5f) >> 0x10;
    temp_s6->u.i[1][0] = (s32)(((-arg3 / sp70) * 65536.0f) + 0.5f) >> 0x10;
    temp_s6->u.f[1][1] = (s32)(((-arg4 / sp70) * 65536.0f) + 0.5f) & 0xFFFF;
    temp_s6->u.f[1][0] = (s32)(((-arg3 / sp70) * 65536.0f) + 0.5f) & 0xFFFF;

    for (i = 0; i < currLod->partCount; i++) {
        currPart = &currLod->partTable[i];

        uvGfx_802235A4(arg0->unk4[i], 1);

        for (j = 0; j < currPart->stateCount; j++) {
            uvGfxStateDraw(&currPart->stateTable[j]);
        }
        // FAKE
        if (currLod) { }
        if (i == currLod->partCount - 1) {
            break;
        }

        temp_v1 = currLod->partTable[i].unk6 - currLod->partTable[i + 1].unk6;
        for (j = 0; j <= temp_v1; j++) {
            uvGfxMtxViewPop();
        }
    }
    temp_v1 = currLod->partTable[currLod->partCount - 1].unk6;
    for (i = 0; i <= temp_v1; i++) {
        uvGfxMtxViewPop();
    }
    temp_s6->m[0][0] = sp88.m[0][0];
    temp_s6->m[0][2] = sp88.m[0][2];
    temp_s6->m[2][0] = sp88.m[2][0];
    temp_s6->m[2][2] = sp88.m[2][2];
}

UnkSobjDraw* uvSobjLoadTerra(u32 soid) {
    UnkSobjDraw* temp_a0;
    ParsedUVTR* uvtr;
    uvUnkTileStruct* temp_v1;

    uvtr = gGfxUnkPtrs->terras[(soid >> 0x18) & 0xFF];
    if (uvtr == NULL) {
        _uvDebugPrintf("Sobj: terra %d not in level\n", (soid >> 0x18) & 0xFF);
        return NULL;
    }

    temp_v1 = &uvtr->unk28[(soid >> 0xC) & 0xFFF];
    if (temp_v1 == NULL) {
        _uvDebugPrintf("Sobj: bad soid 0x%x, tile not found in terra\n", soid);
        return NULL;
    }

    temp_a0 = &temp_v1->unk40->unk10[soid & 0xFFF];
    if (temp_a0 == NULL) {
        _uvDebugPrintf("Sobj: bad soid, sobj %d not in terra %d, tile %d\n", (soid >> 0x18) & 0xFF, (soid >> 0xC) & 0xFFF, soid & 0xFFF);
        return NULL;
    }
    return temp_a0;
}

void uvSobjPosm(u32 soid, s32 dstIdx, Mtx4F* src) {
    UnkSobjDraw* temp_v0;
    f32 temp_fa0;
    f32 temp_fa1;
    f32 temp_ft4;

    temp_v0 = uvSobjLoadTerra(soid);
    if (temp_v0 == NULL) {
        return;
    }

    temp_fa0 = src->m[3][0] / gGfxUnkPtrs->unk1608;
    temp_fa1 = src->m[3][1] / gGfxUnkPtrs->unk1608;
    temp_ft4 = src->m[3][2] / gGfxUnkPtrs->unk1608;

    if (dstIdx == 0 && (FABS(temp_fa0 - temp_v0->unk8) > 0.01f || FABS(temp_fa1 - temp_v0->unkC) > 0.01f || FABS(temp_ft4 - temp_v0->unk10) > 0.01f)) {
        _uvDebugPrintf("uvSobjPosm: warning, trying to translate part 0\n");
        _uvDebugPrintf("            new %f , %f , %f\n", temp_fa0, temp_fa1, temp_ft4);
        _uvDebugPrintf("            old %f , %f , %f\n", temp_v0->unk8, temp_v0->unkC, temp_v0->unk10);
    } else {
        uvMat4CopyF2L(&temp_v0->unk4[dstIdx], src);
    }
}

void uvSobj_8022D168(u32 soid, s32 srcIdx, Mtx4F* dst) {
    UnkSobjDraw* temp_v0;

    temp_v0 = uvSobjLoadTerra(soid);
    if (temp_v0 != NULL) {
        uvMat4CopyL2F(dst, temp_v0->unk4[srcIdx]);
    }
}

u16 uvSobj_8022D1E4(u32 soid) {
    UnkSobjDraw* temp_v0;

    temp_v0 = uvSobjLoadTerra(soid);
    if (temp_v0 == NULL) {
        return 0xFFFF;
    }

    return temp_v0->modelId;
}

void uvSobjModel(u32 soid, s32 modelId) {
    UnkSobjDraw* temp_v0;
    ParsedUVMD* uvmd;

    temp_v0 = uvSobjLoadTerra(soid);
    if (temp_v0 == NULL) {
        return;
    }
    uvmd = gGfxUnkPtrs->models[modelId];
    if (uvmd == NULL) {
        _uvDebugPrintf("uvSobjModel: model %d not in level\n", modelId);
        return;
    }
    if (uvmd->lodTable->partCount != gGfxUnkPtrs->models[temp_v0->modelId]->lodTable->partCount) {
        _uvDebugPrintf("uvSobjModel: new model %d had different  heirarchy\n", modelId);
        return;
    }
    temp_v0->modelId = modelId;
}
