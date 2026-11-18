#include "common.h"
#include <uv_filesystem.h>
#include <uv_graphics.h>
#include <uv_math.h>
#include <uv_matrix.h>
#include <uv_memory.h>
#include "ball_target.h"
#include "balls.h"
#include "boats.h"
#include "bonus.h"
#include "code_72910.h"
#include "code_82520.h"
#include "code_9A960.h"
#include "code_B2900.h"
#include "env_sound.h"
#include "environment.h"
#include "falco.h"
#include "ferry.h"
#include "fountain.h"
#include "game.h"
#include "glider_toys.h"
#include "hover_pads.h"
#include "jumble_hopper.h"
#include "level.h"
#include "mem.h"
#include "missi.h"
#include "oil_plant.h"
#include "pads.h"
#include "planes.h"
#include "rings.h"
#include "shuttle.h"
#include "skydiving.h"
#include "snap.h"
#include "snd.h"
#include "targets.h"
#include "task.h"
#include "thermals.h"
#include "wind.h"
#include "whale.h"
#include "whale_pod.h"

typedef struct {
    void* taskName; // short string name "BIRD 1C"
    void* taskId;   // string identifier "E_BD_3"
    u8 taskIdx;
    u8 pad9[0x3];
} Unk803798E0;

STATIC_DATA u8 sTaskClassU8 = 0;
STATIC_DATA u8 sTaskVehicleU8 = 0;
STATIC_DATA u8 sTaskTestU8 = 0;
STATIC_DATA u8 D_8035079C = 0;
u8 D_803507A0 = FALSE;
STATIC_DATA u8 D_803507A4 = 0;
STATIC_DATA u8 sTaskMapLookup[] = { MAP_HOLIDAY_ISLAND, MAP_CRESCENT_ISLAND, MAP_LITTLE_STATES, MAP_EVER_FROST_ISLAND };
STATIC_DATA u8 sTaskUserFileIdxLookup[] = {
    0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,
    0x3E, 0x3F, 0x40, 0x41, 0x11, 0x12, 0x13, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// level data of some sorts
STATIC_DATA Unk803798E0 D_803798E0[MAX_CLASSES][MAX_TESTS][VEHICLE_COUNT];
STATIC_DATA TaskObjects sTaskObjects;

// begins 0x8037AA78
s32 sTaskClass; // ClassId
s32 sTaskTest;
s32 sTaskVehicle; // VehicleId
TaskTPAD sTaskTakeoffPads[4];

// forward declarations
void taskDeinit(void);

void taskInit(void) {
    s32 classIdx, testIdx, vehIdx;
    TaskObjects* obj;
    Unk803798E0* taskInfo;
    s32 i;
    u8 commName[0x28];
    u8 taskName[0x28];

    thermInit();
    ringsInit();
    windInit();
    bonusInit();
    ballsInit();
    padsInit();
    targetsInit();
    hoverPadInit();
    ballTgtInit();
    snapInit();
    falcoInit();
    func_802EB3E0();
    skydivingInit();
    func_802FAF80();
    shuttleInit();
    ferryInit();
    fountainInit();
    missiInit();
    whaleInit();
    oilPlantInit();
    gliderToyInit();
    boatsInit();
    planesInit();
    whalePodInit();

    for (classIdx = 0; classIdx < MAX_CLASSES; classIdx++) {
        for (testIdx = 0; testIdx < MAX_TESTS; testIdx++) {
            for (vehIdx = 0; vehIdx < VEHICLE_COUNT; vehIdx++) {
                D_803798E0[classIdx][testIdx][vehIdx].taskIdx = 0xFF;
                D_803798E0[classIdx][testIdx][vehIdx].taskName = NULL;
                D_803798E0[classIdx][testIdx][vehIdx].taskId = NULL;
            }
        }
    }

    for (i = 0; i < 0x3D; i++) {
        obj = taskLoadCommObj(i);
        classIdx = obj->comm.classNum;
        testIdx = obj->comm.testNum;
        vehIdx = obj->comm.vehNum;
        _uvMediaCopy(commName, (void*)obj->dataNAME, 0x28);
        if (classIdx >= MAX_CLASSES) {
            _uvDebugPrintf("\ntask : level index out of range - current limit %d\n", MAX_CLASSES - 1);
        }
        if (testIdx >= MAX_TESTS) {
            _uvDebugPrintf("\ntask : stage index out of range - current limit %d\n", MAX_TESTS - 1);
        }
        if (vehIdx >= VEHICLE_COUNT) {
            _uvDebugPrintf("\ntask : vehicle index out of range - current limit %d\n", VEHICLE_COUNT - 1);
        }
        taskInfo = &D_803798E0[classIdx][testIdx][vehIdx];
        if (taskInfo->taskIdx != 0xFF) {
            _uvMediaCopy(taskName, taskInfo->taskName, sizeof(taskName));
            _uvDebugPrintf("task : oops! redefining [%s] idx:%d veh:%d stage:%d lev:%d -> [%s]\n", taskName, taskInfo->taskIdx, vehIdx, testIdx, classIdx,
                           commName);
        }
        taskInfo->taskIdx = i;
        taskInfo->taskName = obj->dataNAME;
        taskInfo->taskId = obj->dataJPTX;
        if ((vehIdx == VEHICLE_BIRDMAN) && (testIdx == 0)) {
            _uvMediaCopy(&sTaskTakeoffPads[classIdx], obj->dataTPAD, sizeof(TaskTPAD));
        }
    }
}

s32 taskIsValidIndex(s32 classIdx, s32 testIdx, s32 vehicle) {
    if (D_803798E0[classIdx][testIdx][vehicle].taskIdx == 0xFF) {
        return 0;
    }
    return 1;
}

// performs toupper() on an array of characters
void strToUpper(char* str, s32 length) {
    int i;

    for (i = 0; i < length; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - ('a' - 'A');
        }
    }
}

s32 taskLoadNames(s32 classIdx, s32 testIdx, s32 vehicle, char* arg3, char* strId) {
    u8 sp48[0x28];
    u8 sp20[0x28];
    Unk803798E0* sp18;

    sp18 = &D_803798E0[classIdx][testIdx][vehicle];
    if (sp18->taskIdx == 0xFF) {
        return 0;
    }
    _uvMediaCopy(&sp48, sp18->taskName, sizeof(sp48));
    _uvMediaCopy(arg3, arg3, sizeof(sp48)); //! @bug: copy should be sp48->arg3
    _uvMediaCopy(&sp20, sp18->taskId, sizeof(sp20));
    _uvMediaCopy(strId, &sp20, sizeof(sp20));
    strToUpper(arg3, 0x28);
    strToUpper(strId, 0x28);
    return 1;
}

s32 taskGetTestCount(s32 classIdx, s32 vehicle) {
    s32 testCount;
    s32 i;

    testCount = 0;
    if (classIdx > MAX_CLASSES) {
        return 0;
    }
    if (vehicle > VEHICLE_COUNT) {
        return 0;
    }

    for (i = 0; i < MAX_TESTS; i++) {
        if (D_803798E0[classIdx][i][vehicle].taskIdx != 0xFF) {
            testCount++;
        }
    }
    return testCount;
}

s32 taskInitTest(s32 classIdx, s32 vehicle, s32 testIdx, u16* map, u16* terraId, u16* envId) {
    u8 taskIdx;

    sTaskClass = classIdx;
    sTaskVehicle = vehicle;
    sTaskTest = testIdx;

    if (classIdx > MAX_CLASSES) {
        return 0;
    }
    if (vehicle > VEHICLE_COUNT) {
        return 0;
    }
    if (testIdx > MAX_TESTS) {
        return 0;
    }

    taskIdx = D_803798E0[classIdx][testIdx][vehicle].taskIdx;
    if (taskIdx == 0xFF) {
        return 0;
    }
    D_8035078C = taskLoadCommObj(taskIdx);
    sTaskClassU8 = (u8)classIdx;
    sTaskVehicleU8 = (u8)vehicle;
    sTaskTestU8 = (u8)testIdx;

    *map = sTaskMapLookup[D_8035078C->comm.mapId];
    switch (*map) {
    case MAP_CRESCENT_ISLAND:
        *terraId = 1;
        break;
    case MAP_LITTLE_STATES:
        *terraId = 3;
        break;
    case MAP_HOLIDAY_ISLAND:
        *terraId = 0;
        break;
    case MAP_EVER_FROST_ISLAND:
        *terraId = 7;
        break;
    default:
        _uvDebugPrintf("task load : unknown db level [%d]\n", *map);
        *map = 3;
        *terraId = 1;
        break;
    }

    thermInit();
    ringsInit();
    windInit();
    bonusInit();
    ballsInit();
    padsInit();
    targetsInit();
    hoverPadInit();
    ballTgtInit();
    snapInit();
    falcoInit();
    skydivingInit();
    func_802EB3E0();
    func_802FAF80();
    ferryInit();
    fountainInit();
    shuttleInit();
    missiInit();
    oilPlantInit();
    whaleInit();
    gliderToyInit();
    boatsInit();
    planesInit();
    whalePodInit();
    D_8035079C = 1;
    D_803507A0 = FALSE;
    D_80362690->unkC[D_80362690->unk9C].unk8 = 0;
    *envId = envGetCurrentId();
    return 1;
}

void taskLoad(void) {
    u16 veh;

    veh = D_80362690->unkC[D_80362690->unk9C].veh;
    D_8035079C = 1;
    windLoad();
    thermLoad();
    padsLoad();
    envLoad();
    func_802EB598();
    if (!D_803507A0) {
        ringsLoad();
        ballsLoad();
        targetsLoad();
        hoverPadLoad();
        ballTgtLoad();
        falcoLoad();
        func_802FB22C();
        if ((veh != VEHICLE_CANNONBALL) && (veh != VEHICLE_SKY_DIVING)) {
            bonusLoad();
        }
    }
    D_803507A4 = 0;
}

void taskDeinitLevel(void) {
    taskDeinit();
    thermDeinit();
    windDeinit();
    padsDeinit();
    func_802EB5E4();
}

void taskDeinit(void) {
    u16 veh;

    veh = D_80362690->unkC[D_80362690->unk9C].veh;
    if (D_8035079C != 0) {
        ringsDeinit();
        ballsDeinit();
        targetsDeinit();
        hoverPadDeinit();
        ballTgtDeinit();
        snapDeinit();
        falcoDeinit();
        func_802FB518();
        if ((veh != VEHICLE_CANNONBALL) && (veh != VEHICLE_SKY_DIVING)) {
            bonusDeinit();
        }
        D_8035079C = 0;
    }
}

s32 taskFrameUpdate(Mtx4F* arg0, f32 arg1) {
    u16 veh;
    s32 sp18;
    s32 temp_v0;

    veh = D_80362690->unkC[D_80362690->unk9C].veh;
    sp18 = 0;
    therm_8034695C();
    wind_8034D548();
    env_802E15F0();
    ballsFrameUpdate();
    falcoFrameUpdate();
    padsFrameUpdate(arg0);
    targetsFrameUpdate(arg0);
    hoverPadFrameUpdate(arg0);
    if ((veh != VEHICLE_CANNONBALL) && (veh != VEHICLE_SKY_DIVING)) {
        bonusFrameUpdate(arg0);
    }
    func_802EB424(arg0, arg1);
    envSoundFrameUpdate(arg0);
    if (ballTgtInGoal() == 1) {
        D_803507A4 = 1;
        sp18 = 1;
        D_80362690->unkC[0].veh = VEHICLE_ROCKET_BELT;
        if (D_80362690->unkC[0].veh != VEHICLE_HANG_GLIDER) {
            sndSetMusic(BGM_ROCKET_BELT_LAND_OK);
            sndSetMusicState(MUS_STATE_PLAY_SEQ);
            func_8033FCD0(D_80362690->unkC[0].veh);
        }
    }
    if (rings_803243D8(arg0) == 1) {
        D_803507A4 = 1;
        sp18 = 1;
        D_80362690->unkC[0].veh = VEHICLE_ROCKET_BELT;
        if (D_80362690->unkC[0].veh != VEHICLE_HANG_GLIDER) {
            sndSetMusic(BGM_ROCKET_BELT_LAND_OK);
            sndSetMusicState(MUS_STATE_PLAY_SEQ);
            func_8033FCD0(D_80362690->unkC[0].veh);
        }
    }
    if ((D_80362690->unkC[D_80362690->unk9C].veh == VEHICLE_JUMBLE_HOPPER) && (func_802FB308(D_80362690->unkC[D_80362690->unk9C].vehicleData) == 1)) {
        sp18 = 1;
    }
    temp_v0 = task_803456D8(arg0);
    switch (temp_v0) {
    case 1:
        taskUpdateState();
        func_802EDDEC(arg0);
        break;
    case 2:
        func_802EDDEC(arg0);
        break;
    }
    if ((D_80362690->unkA0 != 0) && (taskGet_80346364() == 3) && (func_8032C080(NULL) != 0)) {
        D_803507A0 = TRUE;
        taskDeinit();
        D_80362690->unkA0 = 0;
    }
    return sp18;
}

s32 task_803456D8(Mtx4F* arg0) {
    LevelTPTS* sp8C;
    f32 temp_fv0;
    u8 temp_v0;
    u8 var_s1;
    u8 sp85;
    Vec3F sp78;
    Vec3F sp6C;
    f32 temp_fv1;
    s32 temp_v0_2;
    s32 var_s2;
    LevelTPTS* temp_s0;

    var_s2 = -1;
    temp_v0 = levelGetTPTS(&sp8C);
    if (temp_v0 == 0) {
        return 0;
    }

    for (var_s1 = 0; var_s1 < temp_v0; var_s1++) {
        temp_s0 = &sp8C[var_s1];
        sp78.x = arg0->m[3][0] - temp_s0->unk4_X;
        sp78.y = arg0->m[3][1] - temp_s0->unk8_Y;
        sp78.z = arg0->m[3][2] - temp_s0->unkC_Z;
        if (uvVec3Len(&sp78) <= temp_s0->unk14) {
            sp6C.x = uvCosF((temp_s0->unk10 + 90.0f) * 0.01745329f); // almost DEG_TO_RAD(1)
            sp6C.y = uvSinF((temp_s0->unk10 + 90.0f) * 0.01745329f); // almost DEG_TO_RAD(1)
            sp6C.z = 0.0f;
            temp_fv0 = uvVec3Dot(&sp6C, &sp78);
            if (temp_fv0 > 0.0f) {
                temp_s0->unk1C = D_8034F850;
            } else {
                temp_s0->unk24 = D_8034F850;
            }
            if (temp_s0->unk0 == 0) {
                if (temp_fv0 > 0.0f) {
                    var_s2 = temp_s0->unk18;
                } else {
                    var_s2 = temp_s0->unk20;
                }
                sp85 = var_s1;
            } else {
                temp_fv1 = temp_s0->unk1C;
                if (temp_fv1 > 0.0f) {
                    if ((temp_s0->unk24 > 0.0f) && (temp_s0->unk24 < temp_fv1)) {
                        if (temp_fv0 > 0.0f) {
                            var_s2 = temp_s0->unk18;
                        } else {
                            var_s2 = temp_s0->unk20;
                        }
                        sp85 = var_s1;
                    }
                }
            }
        } else {
            temp_s0->unk24 = 0.0f;
            temp_s0->unk1C = 0.0f;
        }
    }
    if (var_s2 == -1) {
        return 0;
    }
    D_80362690->unkC[D_80362690->unk9C].unk8 = (u16)var_s2;
    temp_v0_2 = task_80346370(var_s2);
    temp_s0 = &sp8C[sp85];
    uvChanTerra(D_80362690->unkC[D_80362690->unk9C].unk70->unk22C, temp_v0_2);
    if (temp_v0_2 != D_80362690->terraId) {
        D_80362690->terraId = (u16)temp_v0_2;
        arg0->m[3][0] += temp_s0->unk28_X;
        arg0->m[3][1] += temp_s0->unk2C_Y;
        arg0->m[3][2] += temp_s0->unk30_Z;
        return 1;
    }
    if ((temp_s0->unk18 == -1) || (temp_s0->unk20 == -1)) {
        arg0->m[3][0] += temp_s0->unk28_X;
        arg0->m[3][1] += temp_s0->unk2C_Y;
        arg0->m[3][2] += temp_s0->unk30_Z;
        return 2;
    }
    return 0;
}

void taskUpdateState(void) {
    u16 veh;

    veh = D_80362690->unkC[D_80362690->unk9C].veh;
    rings_80323364();
    if ((veh != VEHICLE_CANNONBALL) && (veh != VEHICLE_SKY_DIVING)) {
        bonusUpdateState();
    }
    balls_802CAF50();
    hoverPad_80309868();
    ballTgtUpdateState();
    func_802FAFF0();
}

s32 taskGetTHER(TaskTHER** data) {
    *data = D_8035078C->dataTHER;
    return D_8035078C->comm.countTHER;
}

s32 taskGetLWIN(TaskLWIN** data) {
    *data = D_8035078C->dataLWIN;
    return D_8035078C->comm.countLWIN;
}

TaskObjects_Unk10* taskGet_80345AEC(void) {
    return &D_8035078C->comm.unk10;
}

s32 taskGetTPAD(TaskTPAD** data) {
    *data = D_8035078C->dataTPAD;
    return D_8035078C->comm.countTPAD;
}

s32 taskGetCNTG(TaskCNTG** data) {
    *data = D_8035078C->dataCNTG;
    return D_8035078C->comm.countCNTG;
}

s32 taskGetOBSV(TaskOBSV** data) {
    *data = D_8035078C->dataOBSV;
    return D_8035078C->comm.countOBSV;
}

s32 taskGetLPAD(TaskLPAD** data) {
    *data = D_8035078C->dataLPAD;
    return D_8035078C->comm.countLPAD;
}

s32 taskGetLSTP(TaskLSTP** data) {
    *data = D_8035078C->dataLSTP;
    return D_8035078C->comm.countLSTP;
}

s32 taskGetRNGS(TaskRNGS** data) {
    *data = D_8035078C->dataRNGS;
    return D_8035078C->comm.countRNGS;
}

s32 taskGetBALS(TaskBALS** data) {
    *data = D_8035078C->dataBALS;
    return D_8035078C->comm.countBALS;
}

s32 taskGetTARG(TaskTARG** data) {
    *data = D_8035078C->dataTARG;
    return D_8035078C->comm.countTARG;
}

s32 taskGetHPAD(TaskHPAD** data) {
    *data = D_8035078C->dataHPAD;
    return D_8035078C->comm.countHPAD;
}

s32 taskGetBTGT(TaskBTGT** data) {
    *data = D_8035078C->dataBTGT;
    return D_8035078C->comm.countBTGT;
}

s32 taskGetPHTS(TaskPHTS** data) {
    *data = D_8035078C->dataPHTS;
    return D_8035078C->comm.countPHTS;
}

s32 taskGetFALC(TaskFALC** data) {
    if (data != NULL) {
        *data = D_8035078C->dataFALC;
    }
    return D_8035078C->comm.countFALC;
}

Unk80345C80* taskGet_80345C80(void) {
    return &D_8035078C->comm.unk48;
}

TaskObjUnk2C* taskGet_80345C90(void) {
    return &D_8035078C->comm.unk2C;
}

f32 taskGet_80345CA0(void) {
    return D_8035078C->comm.unk44;
}

u8* taskGet_80345CB0(void) {
    return D_8035078C->comm.unk8;
}

void taskGet_80345CC0(f32* arg0, f32* arg1) {
    *arg0 = D_8035078C->comm.unk48.unk3BC;
    *arg1 = D_8035078C->comm.unk48.unk3C0;
}

TaskObjects* taskLoadCommObj(u32 taskIdx) {
    void* sp34;
    s32 idx;
    u32 size;
    u32 tag;
    void* data;
    TaskObjects* dst;

    dst = &sTaskObjects;
    sp34 = uvUserFileRead(sTaskUserFileIdxLookup[taskIdx], MEM_ROM_OFFSET);
    idx = uvFileReadHeader((s32)sp34);
    uvMemSet(&sTaskObjects, 0, sizeof(sTaskObjects));

    while ((tag = uvFileReadBlock(idx, &size, &data, 2)) != 0) {
        switch (tag) {
        case 'JPTX': // 0x4A505458
            dst->dataJPTX = data;
            break;
        case 'NAME': // 0x4E414D45
            dst->dataNAME = data;
            break;
        case 'INFO': // 0x494E464F
            dst->dataINFO = data;
            break;
        case 'COMM': // 0x434F4D4D
            _uvMediaCopy(&dst->comm, data, sizeof(dst->comm));
            mem_init();
            dst->dataTPAD = mem_get(dst->comm.countTPAD * sizeof(TaskTPAD));
            dst->dataLPAD = mem_get(dst->comm.countLPAD * sizeof(TaskLPAD));
            dst->dataLSTP = mem_get(dst->comm.countLSTP * sizeof(TaskLSTP));
            dst->dataLWIN = mem_get(dst->comm.countLWIN * sizeof(TaskLWIN));
            dst->dataRNGS = mem_get(dst->comm.countRNGS * sizeof(TaskRNGS));
            dst->dataTHER = mem_get(dst->comm.countTHER * sizeof(TaskTHER));
            dst->dataBALS = mem_get(dst->comm.countBALS * sizeof(TaskBALS));
            dst->dataTARG = mem_get(dst->comm.countTARG * sizeof(TaskTARG));
            dst->dataHPAD = mem_get(dst->comm.countHPAD * sizeof(TaskHPAD));
            dst->dataBTGT = mem_get(dst->comm.countBTGT * sizeof(TaskBTGT));
            dst->dataPHTS = mem_get(dst->comm.countPHTS * sizeof(TaskPHTS));
            dst->dataFALC = mem_get(dst->comm.countFALC * sizeof(TaskFALC));
            dst->dataCNTG = mem_get(dst->comm.countCNTG * sizeof(TaskCNTG));
            dst->dataSDFM = mem_get(dst->comm.countSDFM * sizeof(TaskSDFM));
            dst->dataHOPD = mem_get(dst->comm.countHOPD * sizeof(TaskHOPD));
            dst->dataOBSV = mem_get(dst->comm.countOBSV * sizeof(TaskOBSV));
            break;
        case 'THER': // 0x54484552
            _uvMediaCopy(dst->dataTHER, data, size);
            break;
        case 'LWIN': // 0x4C57494E
            _uvMediaCopy(dst->dataLWIN, data, size);
            break;
        case 'TPAD': // 0x54504144
            _uvMediaCopy(dst->dataTPAD, data, size);
            break;
        case 'LPAD': // 0x4C504144
            _uvMediaCopy(dst->dataLPAD, data, size);
            break;
        case 'LSTP': // 0x4C535450
            _uvMediaCopy(dst->dataLSTP, data, size);
            break;
        case 'RNGS': // 0x524E4753
            _uvMediaCopy(dst->dataRNGS, data, size);
            break;
        case 'BALS': // 0x42414C53
            _uvMediaCopy(dst->dataBALS, data, size);
            break;
        case 'PHTS': // 0x50485453
            _uvMediaCopy(dst->dataPHTS, data, size);
            break;
        case 'HOPD': // 0x484F5044
            _uvMediaCopy(dst->dataHOPD, data, size);
            break;
        case 'TARG': // 0x54415247
            _uvMediaCopy(dst->dataTARG, data, size);
            break;
        case 'HPAD': // 0x48504144
            _uvMediaCopy(dst->dataHPAD, data, size);
            break;
        case 'BTGT': // 0x42544754
            _uvMediaCopy(dst->dataBTGT, data, size);
            break;
        case 'FALC': // 0x46414C43
            _uvMediaCopy(dst->dataFALC, data, size);
            break;
        case 'CNTG': // 0x434E5447
            _uvMediaCopy(dst->dataCNTG, data, size);
            break;
        case 'SDFM': // 0x5344464D
            _uvMediaCopy(dst->dataSDFM, data, size);
            break;
        case 'OBSV': // 0x4F425356
            _uvMediaCopy(dst->dataOBSV, data, size);
            break;
        }
    }
    uvFile_80223F30(idx);
    return &sTaskObjects;
}

void taskBirdmanPad(u16 mapIdx) {
    if (mapIdx >= ARRAY_COUNT(sTaskTakeoffPads)) {
        _uvDebugPrintf("task_birdpad : level out of valid range [%d] - set to 0\n", mapIdx);
        mapIdx = 0;
    }
    _uvMediaCopy(D_8035078C->dataTPAD, &sTaskTakeoffPads[mapIdx], sizeof(TaskTPAD));
}

void taskGetClsVehTest(u16* classIdx, u16* vehIdx, u16* testIdx) {
    *classIdx = sTaskClass;
    *vehIdx = sTaskVehicle;
    *testIdx = sTaskTest;
}

u8 taskGet_80346364(void) {
    return sTaskObjects.comm.unk4;
}

// task_80346370 does not initialize the return value `ret`
#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsometimes-uninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
s32 task_80346370(s32 terra) {
    s32 ret;
    switch (D_80362690->map) {
    case MAP_CRESCENT_ISLAND:
        switch (terra) {
        case 0:
            ret = 1;
            break;
        case 1:
            ret = 2;
            break;
        case 2:
            ret = 7;
            break;
        default:
            _uvDebugPrintf("task : unknown CISLAND terra selection [%d]\n", terra);
            break;
        }
        break;
    case MAP_LITTLE_STATES:
        ret = 3;
        break;
    case MAP_HOLIDAY_ISLAND:
        ret = 0;
        break;
    case MAP_EVER_FROST_ISLAND:
        switch (terra) {
        case 0:
            ret = 7;
            break;
        case 1:
            ret = 8;
            break;
        default:
            _uvDebugPrintf("task : unknown ARCTIC level terra selection [%d]\n", terra);
            break;
        }
        break;
    default:
        _uvDebugPrintf("task : unknown level for terra selection [%d]\n", D_80362690->map);
        break;
    }
    return ret;
}
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

u8 taskGet_80346468(void) {
    return D_803507A4;
}

s32 taskGetHOPD(TaskHOPD** data) {
    *data = D_8035078C->dataHOPD;
    return D_8035078C->comm.countHOPD;
}
