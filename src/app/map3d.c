#include "common.h"
#include <uv_font.h>
#include <uv_geometry.h>
#include <uv_graphics.h>
#include <uv_math.h>
#include <uv_matrix.h>
#include <uv_sprite.h>
#include <uv_vector.h>
#include "balls.h"
#include "code_D1ED0.h"
#include "demo.h"
#include "falco.h"
#include "ferry.h"
#include "fountain.h"
#include "hover_pads.h"
#include "hud.h"
#include "map3d.h"
#include "missi.h"
#include "oil_plant.h"
#include "rings.h"
#include "shuttle.h"
#include "snd.h"
#include "targets.h"
#include "task.h"
#include "thermals.h"
#include "uv_dobj.h"
#include "whale.h"

u16 sMapDobjId = 0xFFFF;
u8 sMapObjectCount = 0;
u16 sMapMessageCount = 0;
f32 D_8034F7FC = 0.0f;
f32 D_8034F800 = 0.0f;
f32 D_8034F804 = 0.0f;
f32 D_8034F808 = 0.0f;
f32 D_8034F80C = 0.6603981256f;
f32 D_8034F810 = 0.6603981256f;
f32 D_8034F814 = 0.0f;
f32 D_8034F818 = 0.0f;
f32 D_8034F81C = 0.0f;
f32 D_8034F820 = 0.0f;
f32 D_8034F824 = 0.0f;
f32 D_8034F828 = 0.0f;
f32 D_8034F82C = 0.0f;
f32 D_8034F830 = 0.0f;
f32 D_8034F834 = 0.0f;
f32 D_8034F838 = 0.0f;
u8 sMapHudFlagsSave = 0;

STATIC_DATA u8 sMapEmitterDev0;
STATIC_DATA u8 sMapEmitterDev1;
STATIC_DATA u16 sMapObjects[60];
STATIC_DATA Vec3F sMapMessages[20];
STATIC_DATA f32 sMapGlobalScale;

// forward declarations
void map3d_803123C4(f32, f32);
void map3d_80312514(void);
void map3dAddItem(s32, f32 x, f32 y, f32 z);

void map3dMain(Unk80362690* arg0, s32 arg1) {
    map3dLoad(arg0, arg1);
    while (map3dHandler(0, arg1) != 0) {
        uvGfxBegin();
        map3dRender(arg0, arg1);
        uvGfxEnd();
    }
    map3dDeinit(arg0, arg1);
}

void map3dLoad(Unk80362690* arg0, s32 arg1) {
    Mtx4F sp148;
    Camera* camera;
    Unk80362690_Unk0* sp140;
    HUDState* hud;
    TaskTHER* therRef;
    u8 therCount;
    TaskTPAD* tpadRef;
    u8 tpadCount;
    TaskLPAD* lpadRef;
    u8 lpadCount;
    TaskLSTP* lstpRef;
    u8 lstpCount;
    TaskRNGS* rngsRef;
    u8 rngsCount;
    TaskTARG* targRef;
    u8 targCount;
    TaskHPAD* hpadRef;
    u8 hpadCount;
    TaskBTGT* btgtRef;
    u8 btgtCount;
    u8 ballCount;
    TaskCNTG* cntgRef;
    u8 cntgCount;
    f32 temp_fs0;
    f32 temp_fs1;
    f32 temp_fv0_3;
    u8 objId;
    s32 falcoId;
    s32 i;
    TaskObjUnk2C* temp_v0_35;
    f32 spD4;
    f32 spD0;
    f32 spCC;
    Vec3F photoPos;
    TaskHOPD* hopdRef;
    s32 hopdCount;
    TaskPHTS* phtsRef;
    s32 phtsCount;
    TaskFALC* falcRef;
    s32 falcCount;
    f32 temp_fa1;
    f32 spA0;
    f32 sp9C;
    f32 sp98;
    f32 sp94;
    f32 sp90;
    f32 sp8C;
    s32 sp88;
    TaskFALC* falc;

    sMapGlobalScale = 0.1f;
    sp140 = &arg0->unkC[arg0->unk9C];
    camera = sp140->unk70;
    hud = hudGetState();
    sMapHudFlagsSave = hud->renderFlags;
    hud->renderFlags = 0;
    hudText_8031D8E0(-1, 0.0f, 0.0f);
    hudWarningText(-1, 0.0f, 0.0f);
    sMapMessageCount = 0;
    uvChan_80204BD4(camera->unk22C, 1, 1.0f);
    uvChan_80204A8C(camera->unk22C, 3);
    uvChan_80204C94(camera->unk22C, -0.7009346f, 0.7009346f, -0.5f, 0.5f, 1.0f, 2048.0f);
    uvChanEnv(camera->unk22C, 0x17);
    sMapDobjId = uvDobjAllocIdx();
    switch (arg0->map) {
    case MAP_CRESCENT_ISLAND:
        uvDobjModel(sMapDobjId, MODEL_CRESCENT_ISLAND_LOW_POLY);
        D_8034F824 = 600.0f;
        break;
    case MAP_LITTLE_STATES:
        uvDobjModel(sMapDobjId, MODEL_LITTLE_STATES_LOW_POLY);
        D_8034F824 = 700.0f;
        break;
    case MAP_HOLIDAY_ISLAND:
        uvDobjModel(sMapDobjId, MODEL_HOLIDAY_ISLAND_LOW_POLY);
        D_8034F824 = 200.0f;
        break;
    case MAP_EVER_FROST_ISLAND:
        uvDobjModel(sMapDobjId, MODEL_EVER_FROST_ISLAND_LOW_POLY);
        D_8034F824 = 700.0f;
        break;
    }
    uvMat4SetIdentity(&sp148);
    uvDobjPosm(sMapDobjId, 0, &sp148);
    rngsCount = 0;
    if (sp140->veh != VEHICLE_CANNONBALL) {
        therCount = taskGetTHER(&therRef);
        tpadCount = taskGetTPAD(&tpadRef);
        lpadCount = taskGetLPAD(&lpadRef);
        lstpCount = taskGetLSTP(&lstpRef);
        rngsCount = taskGetRNGS(&rngsRef);
        hpadCount = taskGetHPAD(&hpadRef);
        btgtCount = taskGetBTGT(&btgtRef);
        targCount = taskGetTARG(&targRef);
        phtsCount = taskGetPHTS(&phtsRef);
        hopdCount = taskGetHOPD(&hopdRef);
        falcCount = taskGetFALC(&falcRef);
        ballCount = ballsGetCount();
    } else {
        ballCount = 0;
        targCount = 0;
        btgtCount = 0;
        hpadCount = 0;
        lstpCount = 0;
        lpadCount = 0;
        tpadCount = 0;
        therCount = 0;
        falcCount = 0;
        hopdCount = 0;
        phtsCount = 0;
    }
    cntgCount = taskGetCNTG(&cntgRef);
    if (arg1) {
        sp140 = &arg0->unkC[arg0->unk9C];
        objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
        uvDobjModel(objId, MODEL_MAP_STARTING_DIRECTION_ARROW);
        uvMat4Copy(&sp148, &sp140->unk2C);
        spD4 = sp148.m[3][0] *= sMapGlobalScale;
        spD0 = sp148.m[3][1] *= sMapGlobalScale;
        spCC = sp148.m[3][2] *= sMapGlobalScale;
        temp_fa1 = 3.0f * sMapGlobalScale;
        spCC += temp_fa1;
        sp148.m[3][2] += temp_fa1;
        uvDobjPosm(objId, 0, &sp148);
        sMapObjectCount++;
        map3dAddItem(BLIT_WAYPOINT_CURRENT_POSITION, spD4, spD0, spCC);
    }

    if (sp140->veh != VEHICLE_BIRDMAN) {
        if (!D_803507A0) {
            for (i = 0; i < rngsCount; i++) {
                if (arg1 && (gRings[i].consumedType != 0)) {
                    continue;
                }
                if ((sp140->unk8 == rngsRef[i].unk18) && (rngsRef[i].ringSubtype != 2)) {
                    objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
                    if (arg1) {
                        if (gRings[i].isActive != 0) {
                            uvDobjModel(objId, MODEL_MAP_RING_TARGET_YELLOW_CUBE);
                        } else {
                            uvDobjModel(objId, MODEL_MAP_RING_TARGET_BLUE_CUBE);
                        }
                        if ((gRings[i].unk1B9 != 0) && arg1) {
                            map3dAddItem(BLIT_WAYPOINT_NEXT, gRings[i].curPose.m[3][0] * sMapGlobalScale, gRings[i].curPose.m[3][1] * sMapGlobalScale,
                                         gRings[i].curPose.m[3][2] * sMapGlobalScale);
                        } else if (gRings[i].scoreType == 3) {
                            map3dAddItem(BLIT_WAYPOINT_GOAL, gRings[i].curPose.m[3][0] * sMapGlobalScale, gRings[i].curPose.m[3][1] * sMapGlobalScale,
                                         gRings[i].curPose.m[3][2] * sMapGlobalScale);
                        }
                    } else {
                        uvDobjModel(objId, MODEL_MAP_RING_TARGET_YELLOW_CUBE);
                    }
                    uvMat4SetIdentity(&sp148);
                    if (arg1) {
                        sp148.m[3][0] = gRings[i].curPose.m[3][0] * sMapGlobalScale;
                        sp148.m[3][1] = gRings[i].curPose.m[3][1] * sMapGlobalScale;
                        sp148.m[3][2] = gRings[i].curPose.m[3][2] * sMapGlobalScale;
                    } else {
                        sp148.m[3][0] = rngsRef[i].pos.x * sMapGlobalScale;
                        sp148.m[3][1] = rngsRef[i].pos.y * sMapGlobalScale;
                        sp148.m[3][2] = rngsRef[i].pos.z * sMapGlobalScale;
                    }
                    uvDobjPosm(objId, 0, &sp148);
                    sMapObjectCount++;
                    if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                        _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
                    }
                }
            }
        }

        for (i = 0; i < lpadCount; i++) {
            spA0 = lpadRef[i].pos.x * sMapGlobalScale;
            sp9C = lpadRef[i].pos.y * sMapGlobalScale;
            sp98 = lpadRef[i].pos.z * sMapGlobalScale;
            if (arg1 && (uvLength2D(spD4 - spA0, spD0 - sp9C) < 10.0f)) {
                continue;
            }
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            uvDobjModel(objId, MODEL_MAP_TARGET_INDICATOR_ARROW);
            uvMat4SetIdentity(&sp148);
            sp148.m[3][0] = spA0;
            sp148.m[3][1] = sp9C;
            sp148.m[3][2] = sp98;
            if (arg1) {
                uvMat4Scale(&sp148, 0.3f, 0.3f, 0.3f);
            }
            uvDobjPosm(objId, 0, &sp148);
            sMapObjectCount++;
            if (arg1) {
                map3dAddItem(BLIT_WAYPOINT_LANDING_POINT, spA0, sp9C, sp98);
            }
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
        }

        for (i = 0; i < lstpCount; i++) {
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            uvDobjModel(objId, MODEL_MAP_TARGET_INDICATOR_ARROW);
            uvMat4SetIdentity(&sp148);
            sp148.m[3][0] = (lstpRef[i].posUL.x + lstpRef[i].posLR.x) * 0.5f * sMapGlobalScale;
            sp148.m[3][1] = (lstpRef[i].posUL.y + lstpRef[i].posLR.y) * 0.5f * sMapGlobalScale;
            sp148.m[3][2] = (lstpRef[i].posUL.z + lstpRef[i].posLR.z) * 0.5f * sMapGlobalScale;
            if (arg1) {
                uvMat4Scale(&sp148, 0.3f, 0.3f, 0.3f);
            }
            uvDobjPosm(objId, 0, &sp148);
            sMapObjectCount++;
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
            if (arg1) {
                map3dAddItem(BLIT_WAYPOINT_RUNWAY, sp148.m[3][0], sp148.m[3][1], sp148.m[3][2]);
            }
        }

        for (i = 0; i < tpadCount; i++) {
            sp94 = tpadRef[i].pos.x * sMapGlobalScale;
            sp90 = tpadRef[i].pos.y * sMapGlobalScale;
            sp8C = tpadRef[i].pos.z * sMapGlobalScale;
            if ((arg1) && (uvLength2D(spD4 - sp94, spD0 - sp90) < 10.0f)) {
                continue;
            }
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            uvDobjModel(objId, MODEL_MAP_WIND_INDICATOR_ARROW);
            func_80313640(sp94, sp90, sp8C, tpadRef[i].angle.x * 0.0174533f, tpadRef[i].angle.y * 0.0174533f, tpadRef[i].angle.z * 0.0174533f, &sp148);
            if (arg1) {
                uvMat4Scale(&sp148, 0.3f, 0.3f, 0.3f);
            }
            uvDobjPosm(objId, 0, &sp148);
            sMapObjectCount++;
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
        }

        if (gThermReady) {
            for (i = 0; i < therCount; i++) {
                objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
                uvDobjModel(objId, MODEL_MAP_THERMAL_CYLINDER);
                uvMat4SetIdentity(&sp148);
                sp148.m[3][0] = therRef[i].pos.x * sMapGlobalScale;
                sp148.m[3][1] = therRef[i].pos.y * sMapGlobalScale;
                sp148.m[3][2] = therRef[i].pos.z * sMapGlobalScale;
                sp148.m[0][0] = ((2.0f * therRef[i].scale) / 1000.0f) * 4.0f;
                sp148.m[1][1] = ((2.0f * therRef[i].scale) / 1000.0f) * 4.0f;
                sp148.m[2][2] = (therRef[i].height / 1000.0f) * 4.0f;
                uvDobjPosm(objId, 0, &sp148);
                sMapObjectCount++;
                if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                    _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
                }
            }
        }

        for (i = 0; i < hpadCount; i++) {
            if (((arg1) && (gHoverPads[i].wasLandedOn)) || (sp140->unk8 != hpadRef[i].unk18)) {
                continue;
            }
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            if (arg1) {
                if (gHoverPads[i].active != 0) {
                    uvDobjModel(objId, MODEL_MAP_RING_TARGET_YELLOW_CUBE);
                } else {
                    uvDobjModel(objId, MODEL_MAP_RING_TARGET_BLUE_CUBE);
                }
                if ((gHoverPads[i].unk6C == 1) && (arg1)) {
                    map3dAddItem(BLIT_WAYPOINT_NEXT, gHoverPads[i].pose.m[3][0] * sMapGlobalScale, gHoverPads[i].pose.m[3][1] * sMapGlobalScale,
                                 gHoverPads[i].pose.m[3][2] * sMapGlobalScale);
                }
            } else {
                uvDobjModel(objId, MODEL_MAP_RING_TARGET_YELLOW_CUBE);
            }
            uvMat4SetIdentity(&sp148);
            sp148.m[3][0] = hpadRef[i].pos.x * sMapGlobalScale;
            sp148.m[3][1] = hpadRef[i].pos.y * sMapGlobalScale;
            sp148.m[3][2] = hpadRef[i].pos.z * sMapGlobalScale;
            uvDobjPosm(objId, 0, &sp148);
            sMapObjectCount++;
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
        }

        if (!D_803507A0) {
            for (i = 0; i < targCount; i++) {
#if !defined(VERSION_JP)
                if ((arg1) && (sMissileTargets[i].unk46 != 0)) {
                    continue;
                }
#endif
                objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
                uvDobjModel(objId, MODEL_MAP_RING_TARGET_YELLOW_CUBE);
                uvMat4SetIdentity(&sp148);
                sp148.m[3][0] = targRef[i].pos.x * sMapGlobalScale;
                sp148.m[3][1] = targRef[i].pos.y * sMapGlobalScale;
                sp148.m[3][2] = targRef[i].pos.z * sMapGlobalScale;
                uvDobjPosm(objId, 0, &sp148);
                sMapObjectCount++;
                if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                    _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
                }
            }
        }

        for (i = 0; i < phtsCount; i++) {
            switch (phtsRef[i].unk0) {
            case 4:
                sp88 = BLIT_WAYPOINT_WHALE;
                whaleGetPos(&photoPos);
                break;
            case 1:
                sp88 = BLIT_WAYPOINT_SPACE_SHUTTLE;
                shuttle_80335F24(&photoPos);
                break;
            case 2:
                sp88 = BLIT_WAYPOINT_PASSENGER_BOAT;
                ferryGetPos(&photoPos);
                break;
            case 6:
                sp88 = BLIT_WAYPOINT_FLAME;
                oilPlantGetPos(&photoPos);
                break;
            case 3:
                sp88 = BLIT_WAYPOINT_MISSI_THE_MONSTER;
                missiGetPos(&photoPos);
                break;
            case 5:
                sp88 = BLIT_WAYPOINT_FOUNTAIN;
                fountainGetPos(&photoPos);
                break;
            }
            if (arg1) {
                map3dAddItem(sp88, photoPos.x * sMapGlobalScale, photoPos.y * sMapGlobalScale, photoPos.z * sMapGlobalScale);
            }
        }

        for (i = 0; i < hopdCount; i++) {
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            uvDobjModel(objId, MODEL_RED_WHITE_GOAL);
            uvMat4SetIdentity(&sp148);
            sp148.m[0][0] = hopdRef[i].scale;
            sp148.m[1][1] = hopdRef[i].scale;
            sp148.m[2][2] = hopdRef[i].height;
            uvDobjProps(objId, 3, MAX(hopdRef[i].scale, hopdRef[i].height), 0);
            uvMat4Scale(&sp148, sMapGlobalScale, sMapGlobalScale, sMapGlobalScale);
            sp148.m[3][0] = hopdRef[i].pos.x * sMapGlobalScale;
            sp148.m[3][1] = hopdRef[i].pos.y * sMapGlobalScale;
            sp148.m[3][2] = hopdRef[i].pos.z * sMapGlobalScale;
            uvDobjPosm(objId, 0, &sp148);
            if (arg1) {
                map3dAddItem(BLIT_WAYPOINT_GOAL, hopdRef[i].pos.x * sMapGlobalScale, hopdRef[i].pos.y * sMapGlobalScale,
                             (hopdRef[i].pos.z + hopdRef[i].height) * sMapGlobalScale);
            }
            sMapObjectCount++;
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
        }

        falcoId = 0;
        for (i = 0; i < falcCount; i++) {
            falc = &falcRef[i];
            if (falc->unk10 == 0) {
                continue;
            }
            if (arg1) {
                falco_802E64E0(falcoId++, &sp148);
                sp148.m[3][0] *= sMapGlobalScale;
                sp148.m[3][1] *= sMapGlobalScale;
                sp148.m[3][2] *= sMapGlobalScale;
                map3dAddItem(BLIT_WAYPOINT_MECHA_HAWK, sp148.m[3][0], sp148.m[3][1], sp148.m[3][2] + (27.63f * sMapGlobalScale));
                uvMat4Scale(&sp148, sMapGlobalScale, sMapGlobalScale, sMapGlobalScale);
            } else {
                uvMat4SetIdentity(&sp148);
                uvMat4Scale(&sp148, sMapGlobalScale, sMapGlobalScale, sMapGlobalScale);
                sp148.m[3][0] = falc->unk0.x * sMapGlobalScale;
                sp148.m[3][1] = falc->unk0.y * sMapGlobalScale;
                sp148.m[3][2] = falc->unk0.z * sMapGlobalScale;
            }
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            sMapObjectCount++;
            uvDobjModel(objId, MODEL_MECCA_HAWK);
            uvDobjPosm(objId, 0, &sp148);
        }

        for (i = 0; i < btgtCount; i++) {
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            uvDobjModel(objId, MODEL_GREEN_BLUE_GOAL_0);
            uvMat4SetIdentity(&sp148);
            sp148.m[0][0] = btgtRef[i].radius;
            sp148.m[1][1] = btgtRef[i].radius;
            sp148.m[2][2] = btgtRef[i].height;
            uvDobjProps(objId, 3, MAX(btgtRef[i].radius, btgtRef[i].height), 0);
            uvMat4Scale(&sp148, sMapGlobalScale, sMapGlobalScale, sMapGlobalScale);
            sp148.m[3][0] = btgtRef[i].pos.x * sMapGlobalScale;
            sp148.m[3][1] = btgtRef[i].pos.y * sMapGlobalScale;
            sp148.m[3][2] = btgtRef[i].pos.z * sMapGlobalScale;
            uvDobjPosm(objId, 0, &sp148);
            if (arg1) {
                map3dAddItem(BLIT_WAYPOINT_GOAL, btgtRef[i].pos.x * sMapGlobalScale, btgtRef[i].pos.y * sMapGlobalScale,
                             (btgtRef[i].pos.z + btgtRef[i].height) * sMapGlobalScale);
            }
            sMapObjectCount++;
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
        }

        for (i = 0; i < ballCount; i++) {
            if (gBalls[i].hasPopped) {
                continue;
            }
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            uvDobjModel(objId, MODEL_MAP_RING_TARGET_YELLOW_CUBE);
            uvMat4SetIdentity(&sp148);
            sp148.m[3][0] = gBalls[i].pose.m[3][0] * sMapGlobalScale;
            sp148.m[3][1] = gBalls[i].pose.m[3][1] * sMapGlobalScale;
            sp148.m[3][2] = gBalls[i].pose.m[3][2] * sMapGlobalScale;
            uvDobjPosm(objId, 0, &sp148);
            sMapObjectCount++;
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
        }

        for (i = 0; i < cntgCount; i++) {
            objId = sMapObjects[sMapObjectCount] = uvDobjAllocIdx();
            uvDobjModel(objId, MODEL_CANNONB_TARGET_1);
            func_80313640(cntgRef[i].pos.x, cntgRef[i].pos.y, cntgRef[i].pos.z, cntgRef[i].angle.x * 0.01745329f, cntgRef[i].angle.y * 0.01745329f,
                          cntgRef[i].angle.z * 0.01745329f, &sp148);
            sp148.m[3][0] *= sMapGlobalScale;
            sp148.m[3][1] *= sMapGlobalScale;
            sp148.m[3][2] *= sMapGlobalScale;
            uvDobjPosm(objId, 0, &sp148);
            sMapObjectCount += 1;
            if (arg1) {
                map3dAddItem(BLIT_WAYPOINT_TARGET, sp148.m[3][0], sp148.m[3][1], sp148.m[3][2]);
            }
            if (sMapObjectCount >= ARRAY_COUNT(sMapObjects)) {
                _uvDebugPrintf("map3d : too many map objects [%d]\n", sMapObjectCount);
            }
        }
    }

    D_8034F820 = 0.0f;
    D_8034F81C = D_8034F820;
    D_8034F80C = 0.6603981f;
    D_8034F810 = D_8034F80C;
    if (arg1) {
        map3d_80312514();
    } else {
        temp_v0_35 = taskGet_80345C90();
        D_8034F828 = (temp_v0_35->unk0.x + temp_v0_35->unkC.x) * (0.5f / 10.0f);
        D_8034F830 = (temp_v0_35->unk0.y + temp_v0_35->unkC.y) * (0.5f / 10.0f);
        D_8034F838 = (temp_v0_35->unk0.z + temp_v0_35->unkC.z) * (0.5f / 10.0f);
        temp_fs0 = (temp_v0_35->unkC.x - temp_v0_35->unk0.x) * 0.1f;
        temp_fs1 = (temp_v0_35->unkC.y - temp_v0_35->unk0.y) * 0.1f;
        temp_fv0_3 = (temp_v0_35->unkC.z - temp_v0_35->unk0.z) * 0.1f;
        D_8034F800 = uvSqrtF(SQ(temp_fs0) + SQ(temp_fs1) + SQ(temp_fv0_3));
        D_8034F804 = 0.5f * D_8034F800;
        D_8034F7FC = D_8034F800;
        D_8034F808 = D_8034F7FC;
        temp_fv0_3 = taskGet_80345CA0();
        if (temp_fv0_3 == 0.0f) {
            if (temp_fs1 < temp_fs0) {
                D_8034F818 = 1.5707963f;
            } else {
                D_8034F818 = 0.0f;
            }
        } else {
            D_8034F818 = temp_fv0_3 * 0.0174533f;
        }
        if (!(D_8034F7FC >= 1.0f)) {
#if defined(VERSION_JP)
            _uvAssertMsg("camr >= 1.0f", "map3d.c", 870);
#else // VERSION_US
            _uvAssertMsg("camr >= 1.0f", "map3d.c", 874);
#endif
        }
    }
    D_8034F82C = D_8034F828;
    D_8034F834 = D_8034F830;
    sMapEmitterDev0 = sndMakeDev(SFX_UI_MAP_MOVE);
    sMapEmitterDev1 = sndMakeDev(0x03);
}

s32 map3dHandler(u8 arg0, s32 arg1) {
    f32 sp3C;
    f32 sp38;
    f32 sp34;
    f32 sp30;
    f32 sp2C;
    f32 temp_fv0;
    f32 temp_fv0_2;
    s32 var_v0;

    demo_80323020();
    func_80313D74();
    sp38 = demoGetInputs(D_80362690->unk9C, INPUT_AXIS_X);
    sp34 = demoGetInputs(D_80362690->unk9C, INPUT_AXIS_Y);
    if (demoButtonCheck(D_80362690->unk9C, Z_TRIG) != 0) {
        var_v0 = TRUE;
    } else {
        var_v0 = FALSE;
    }
    if (ABS_NOEQ(sp38) < 0.08f) {
        sp38 = 0.0f;
    }
    if (ABS_NOEQ(sp34) < 0.08f) {
        sp34 = 0.0f;
    }
    if (var_v0) {
        D_8034F81C = 1.5f * sp38;
    } else {
        D_8034F81C = 0.0f;
    }
    sp30 = D_8034F818;
    D_8034F818 += D_8034F81C * D_8034F854;
    if (var_v0) {
        D_8034F814 = 0.95f * sp34;
    } else {
        D_8034F814 = 0.0f;
    }
    sp2C = D_8034F810;
    // clang-format off
    if (sp2C < 0.05f) { sp2C = 0.05f; } else if (sp2C > 1.3707962f) { sp2C = 1.3707962f;}
    // clang-format on
    D_8034F810 += D_8034F814 * D_8034F854;
    if (D_8034F810 < 0.05f) {
        D_8034F810 = 0.05f;
    } else if (D_8034F810 > 1.3707962f) {
        D_8034F810 = 1.3707962f;
    }
    D_8034F80C = D_8034F810;
    if (!var_v0) {
        temp_fv0 = 150.0f * sp38 * sp38;
        if (sp38 < 0.0f) {
            temp_fv0 = -temp_fv0;
        }
        temp_fv0_2 = 150.0f * sp34 * sp34;
        if (sp34 < 0.0f) {
            temp_fv0_2 = -temp_fv0_2;
        }
        map3d_803123C4(temp_fv0, temp_fv0_2);
        func_8033F904(sMapEmitterDev1, 1.0f, 1.0f, 0.0f);
    }
    if (demoButtonCheck(D_80362690->unk9C, U_CBUTTONS) != 0) {
        func_8033F904(sMapEmitterDev1, 1.0f, 1.0f, 0.0f);
        D_8034F808 -= 2000.0f * sMapGlobalScale * D_8034F854;
    } else if (demoButtonCheck(D_80362690->unk9C, D_CBUTTONS) != 0) {
        func_8033F904(sMapEmitterDev1, 1.0f, 1.0f, 0.0f);
        D_8034F808 += 2000.0f * sMapGlobalScale * D_8034F854;
    } else {
        if ((arg1) && (demoButtonCheck(D_80362690->unk9C, A_BUTTON) != 0)) {
            map3d_80312514();
        } else {
            func_8033F904(sMapEmitterDev1, 1.0f, 0.0f, 0.0f);
        }
    }
    if (D_8034F808 < 50.0f) {
        D_8034F808 = 50.0f;
    } else if (D_8034F824 < D_8034F808) {
        D_8034F808 = D_8034F824;
    }
    sp3C = D_8034F808;
    if ((D_8034F7FC == 50.0f) || (D_8034F7FC == D_8034F824)) {
        func_8033F904(sMapEmitterDev1, 1.0f, 0.0f, 0.0f);
    }
    if ((D_8034F82C != D_8034F828) || (D_8034F830 != D_8034F834) || (sp2C != D_8034F810) || (sp30 != D_8034F818)) {
        func_8033F904(sMapEmitterDev0, 1.0f, 0.35f, 0.0f);
    } else {
        func_8033F904(sMapEmitterDev0, 1.0f, 0.0f, 0.0f);
    }
    D_8034F828 = D_8034F82C;
    D_8034F830 = D_8034F834;
    D_8034F7FC = sp3C;
    if (demoButtonPress(arg0, A_BUTTON) != 0) {
        func_8033F904(sMapEmitterDev1, 1.0f, 0.0f, 0.0f);
        func_8033F904(sMapEmitterDev0, 1.0f, 0.0f, 0.0f);
        sndPlaySfx(SFX_UI_CONFIRM);
    }
    if ((demoButtonPress(arg0, START_BUTTON) != 0) || (demoButtonPress(arg0, B_BUTTON) != 0)) {
        func_8033F904(sMapEmitterDev1, 1.0f, 0.0f, 0.0f);
        func_8033F904(sMapEmitterDev0, 1.0f, 0.0f, 0.0f);
        sndPlaySfx(SFX_UI_CANCEL);
        return FALSE;
    } else {
        return TRUE;
    }
}

void map3dRender(Unk80362690* arg0, s32 arg1) {
    Camera* camera;
    Mtx4F sp1BC;
    Mtx4F sp17C;
    Mtx4F sp13C;
    Mtx4F spFC;
    s32 i;
    f32 temp_fs0;
    f32 temp_fs1;
    s32 width;
    s32 temp_ft2;
    s32 temp_ft4;
    s32 offset;
    f32 tempZ;
    f32 x, y, z, w;
    Vec4F spBC;
    Vec4F spAC;

    camera = arg0->unkC[arg0->unk9C].unk70;
    uvMat4SetIdentity(&sp1BC);
    uvMat4LocalTranslate(&sp1BC, D_8034F828, D_8034F830, D_8034F838);
    uvMat4SetIdentity(&sp17C);
    func_803134D0(D_8034F7FC, D_8034F818, D_8034F80C, &sp17C.m[3][0], &sp17C.m[3][1], &sp17C.m[3][2]);
    sp17C.m[3][0] += D_8034F828;
    sp17C.m[3][1] += D_8034F830;
    sp17C.m[3][2] += D_8034F838;
    uvMat4UnkOp6(&camera->unk108, &sp1BC, &sp17C);
    uvChan_80204C94(camera->unk22C, -0.7009346f, 0.7009346f, -1.0f / 2.0f /*-0.5f*/, 1.0f / 2.0f /*0.5f*/, 1.0f, (D_8034F7FC * 1.2f) + 2048.0f);
    uvChan_80204B34(camera->unk22C, &camera->unk108);
    uvChan_80204FC4(camera->unk22C);
    uvFontSet(3);
    uvFontColor(0x64, 0xC8, 0x96, 0xFF);
    uvFontScale(1.0, 1.0);
    if (arg1) {
        uvChan_80205724(camera->unk22C, 2, &sp13C);
        uvChan_80205724(camera->unk22C, 3, &spFC);
        uvMat4SetIdentity(&sp17C);
        uvMat4RotateAxis(&sp17C, -1.5707961f, 0x78);
        uvMat4Mul(&sp1BC, &sp17C, &sp13C);
        uvMat4Mul(&sp17C, &spFC, &sp1BC);
        func_80314154();

        for (i = 0; i < sMapMessageCount; i++) {
            x = (sMapMessages[i].x - camera->unk108.m[3][0]);
            y = (sMapMessages[i].y - camera->unk108.m[3][1]);
            z = (sMapMessages[i].z - camera->unk108.m[3][2]);
            if (((x * camera->unk108.m[1][0]) + (y * camera->unk108.m[1][1]) + (z * camera->unk108.m[1][2])) < 0.0f) {
                continue;
            }
            width = uvSprtGetWidth(20 - i);
            spAC.x = sMapMessages[i].x;
            spAC.y = sMapMessages[i].y;
            spAC.z = sMapMessages[i].z;
            spAC.w = 1.0f;
            func_8034AC34(&sp17C, &spBC, &spAC);
            temp_fs0 = spBC.f[0] / spBC.f[3];
            temp_fs1 = spBC.f[1] / spBC.f[3];
            tempZ = spBC.f[2];
            temp_fs0 = ((temp_fs0 + 1.0f) * 320.0f * 0.5f) + 0.5f;
            temp_fs1 = ((temp_fs1 + 1.0f) * 240.0f * 0.5f) + 0.5f;
            offset = 15;
            temp_ft2 = (s32)(temp_fs0 - (width / 2));
            if ((temp_ft2 > -width) && (temp_ft2 < (camera->viewX1 + width)) && (temp_fs1 > 0.0f) && (temp_fs1 < 240.0f) && (tempZ > 0.0f)) {
                uvVtxBeginPoly();
                temp_ft4 = (s32)(temp_fs1 + offset);
                uvVtx(temp_ft2 - 5, temp_ft4 + offset + 3, 0, 0, 0, 0x00, 0x00, 0x00, 0x80);
                uvVtx(temp_ft2 - 5, temp_ft4 - 3, 0, 0, 0, 0x00, 0x00, 0x00, 0x80);
                uvVtx(temp_ft2 + width + 5, temp_ft4 - 3, 0, 0, 0, 0x00, 0x00, 0x00, 0x80);
                uvVtx(temp_ft2 + width + 5, temp_ft4 + offset + 3, 0, 0, 0, 0x00, 0x00, 0x00, 0x80);
                uvVtxEndPoly();
                uvVtxBeginPoly();
                uvVtx(temp_fs0 - 5, temp_ft4 - 3, tempZ, 0, 0, 0x00, 0x00, 0x00, 0x80);
                uvVtx(temp_fs0, temp_fs1 + 3, tempZ, 0, 0, 0x00, 0x00, 0x00, 0x80);
                uvVtx(temp_fs0 + 5, temp_ft4 - 3, tempZ, 0, 0, 0x00, 0x00, 0x00, 0x80);
                uvVtxEndPoly();
                uvSprtProps(20 - i, SPRT_PROP_POS(temp_ft2, temp_ft4 + offset - 1), SPRT_PROP_END);
                uvGfxStatePush();
                uvSprtDraw(20 - i);
                uvGfxStatePop();
            }
        }
        func_803141E4();
    }
}

void map3dDeinit(Unk80362690* arg0, s32 arg1) {
    Camera* camera;
    s32 i;

    camera = arg0->unkC[arg0->unk9C].unk70;
    uvDobjModel(sMapDobjId, 0xFFFF);
    sMapDobjId = 0xFFFF;
    for (i = 0; i < sMapObjectCount; i++) {
        uvDobjModel(sMapObjects[i], 0xFFFF);
        sMapObjects[i] = 0xFFFF;
    }
    sMapObjectCount = 0;
    func_8033F8CC(sMapEmitterDev0);
    func_8033F8CC(sMapEmitterDev1);
    if (arg1) {
        uvChan_80204A8C(camera->unk22C, 3);
    }
    hudGetState()->renderFlags = sMapHudFlagsSave;
    camera_802D45C4(camera, -1.0f);
}

void map3d_803123C4(f32 arg0, f32 arg1) {
    Camera* camera;
    f32 sp58;
    f32 sp54;
    f32 sp50;
    f32 sp4C;
    f32 cosX;
    f32 sinX;
    f32 pad;
    f32 sp3C;
    f32 ft5;
    f32 mag;

    camera = D_80362690->unkC[D_80362690->unk9C].unk70;
    func_80313570(&camera->unk108, &sp58, &sp58, &sp58, &sp54, &sp50, &sp4C);
    cosX = uvCosF(sp54);
    sinX = uvSinF(sp54);
    ft5 = ((cosX * arg0) - (sinX * arg1)) * D_8034F854;
    sp3C = ((sinX * arg0) + (cosX * arg1)) * D_8034F854;
    D_8034F82C += ft5;
    D_8034F834 += sp3C;
    mag = uvSqrtF(SQ(D_8034F82C) + SQ(D_8034F834));
    if (mag > 1500.0f) {
        D_8034F82C = D_8034F828;
        D_8034F834 = D_8034F830;
    }
}

void map3d_80312514(void) {
    f32 sp44;
    f32 sp40;
    f32 sp3C;
    f32 sp38;
    f32 sp34;
    f32 sp30;
    Unk80362690_Unk0* unkC;

    D_8034F820 = 0.0f;
    D_8034F81C = D_8034F820;
    D_8034F80C = 0.6603981f;

    unkC = &D_80362690->unkC[D_80362690->unk9C];
    func_80313570(&unkC->unk2C, &sp38, &sp34, &sp30, &sp44, &sp40, &sp3C);
    D_8034F82C = sp38 * sMapGlobalScale;
    D_8034F828 = D_8034F82C;
    D_8034F834 = sp34 * sMapGlobalScale;
    D_8034F830 = D_8034F834;
    D_8034F838 = sp30 * sMapGlobalScale;
    D_8034F7FC = 50.0f;
    D_8034F800 = D_8034F7FC;
    D_8034F804 = D_8034F800 * 0.85;
    D_8034F818 = sp44 + 4.712389f;
    D_8034F820 = 0.0f;
    D_8034F808 = D_8034F7FC;
    D_8034F810 = D_8034F80C;
    D_8034F814 = 0.0f;
}

void map3dAddItem(s32 blitId, f32 x, f32 y, f32 z) {
    if (sMapMessageCount >= ARRAY_COUNT(sMapMessages)) {
        _uvDebugPrintf("Map3d: No more message slots (%d)\n", ARRAY_COUNT(sMapMessages));
        return;
    }
    // clang-format off
    uvSprtProps(ARRAY_COUNT(sMapMessages) - sMapMessageCount,
        SPRT_PROP_ENABLED(TRUE),
        SPRT_PROP_BLIT(blitId),
        SPRT_PROP_END
    );
    // clang-format on
    sMapMessages[sMapMessageCount].x = x;
    sMapMessages[sMapMessageCount].y = y;
    sMapMessages[sMapMessageCount].z = z;
    sMapMessageCount++;
}
