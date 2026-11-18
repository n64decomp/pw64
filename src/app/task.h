#ifndef APP_TASK_H
#define APP_TASK_H

#include <PR/ultratypes.h>
#include <uv_matrix.h>
#include <uv_vector.h>

// Relationships between Class, Vehicle, Test, Target, Time-of-Day and the in-game values
// Vehicle: main and bonus vehicles
// Class: code refers "level" | Beg./A/B/Pilot | Bonus Level 1/2/3 | Birdman map
// Test: code refers "stage" | Test# within class | Cannonball target # | Birdman time-of-day
//  Vehicle─▶      0           1          2                3          4          5           6
//           ┌───────────┬───────────┬───────────┐   ┌───────────┬──────────┬──────────┐┌─────────┐
//Class      │Hang Glider│Rocket Pack│Gyrocopter │   │Cannonball │Sky Diving│J. Hopper ││ Birdman │
// ▼┌────────┼───────────┼───────────┼───────────┤┌──┼───────────┼──────────┼──────────┤├─────────┼─────────┐
// 0│Beginner│Test 1     │Test 1     │Test 1     ││L1│Tgt 1,2,3,4│T1        │T1        ││Day,Ev,Nt│Holiday  │
//  ├────────┼───────────┼───────────┼───────────┤├──┼───────────┼──────────┼──────────┤├─────────┼─────────┤
// 1│Class A │Test 1,2   │Test 1,2   │Test 1,2   ││L2│Tgt 1,2,3,4│T1        │T1        ││Day,Ev,Nt│Crescent │
//  ├────────┼───────────┼───────────┼───────────┤├──┼───────────┼──────────┼──────────┤├─────────┼─────────┤
// 2│Class B │Test 1,2,3 │Test 1,2,3 │Test 1,2,3 ││L3│Tgt 1,2,3,4│T1        │T1        ││Day,Ev,Nt│L. States│
//  ├────────┼───────────┼───────────┼───────────┤└──┴───────────┴──────────┴──────────┘├─────────┼─────────┤
// 3│Pilot   │Test 1,2,3 │Test 1,2,3 │Test 1,2,3 │                                      │Day,Ev,Nt│Everfrost│
//  └────────┴───────────┴───────────┴───────────┘                                      └─────────┴─────────┘

#define MAX_CLASSES  8 // max classes for some static structs, this is > CLASS_COUNT
#define MAX_TESTS    5 // max tests per stage for some structs, this is > real count of 4

enum PilotId {
    PILOT_LARK  = 0,
    PILOT_GOOSE = 1,
    PILOT_HAWK  = 2,
    PILOT_KIWI  = 3,
    PILOT_IBIS  = 4,
    PILOT_ROBIN = 5,
    PILOT_COUNT
};

enum VehicleId {
    VEHICLE_HANG_GLIDER = 0,
    VEHICLE_ROCKET_BELT = 1,
    VEHICLE_GYROCOPTER = 2,
    VEHICLE_CANNONBALL = 3,
    VEHICLE_SKY_DIVING = 4,
    VEHICLE_JUMBLE_HOPPER = 5,
    VEHICLE_BIRDMAN = 6,
    VEHICLE_COUNT
};

#define IS_MAIN_VEHICLE(veh) ((veh) <= VEHICLE_GYROCOPTER)
#define IS_BONUS_VEHICLE(veh) (!IS_MAIN_VEHICLE(veh))

enum ClassId {
    CLASS_BEGINNER = 0,
    CLASS_A = 1,
    CLASS_B = 2,
    CLASS_PILOT = 3,
    CLASS_COUNT
};

enum MapId {
    MAP_HOLIDAY_ISLAND = 1,
    MAP_CRESCENT_ISLAND = 3,
    MAP_LITTLE_STATES = 5,
    MAP_EVER_FROST_ISLAND = 10
};

typedef struct {
    Vec3F pos;
    u8 padC[0x18 - 0xC];
    s32 unk18;
    f32 unk1C;
    u8 type;
    u8 willSplit;
    u8 pad22[2];
    f32 unk24;
    f32 unk28;
    f32 unk2C;
    f32 scale;
    s32 points;
    s32 unk38; // copied, but unused
    f32 dragCoef;
    f32 gravity;
    u8 unk44;
    u8 pad45[3];
    f32 unk48;
    f32 unk4C;
    f32 unk50;
    s32 unk54;
    s32 unk58;
    f32 unk5C;
    f32 unk60;
    f32 splitBallGravity;
} TaskBALS; // size = 0x68

typedef struct {
    Vec3F pos;
    s32 terra;
    f32 radius;
    f32 height;
    u8 unk18;
    u8 goalPoints;
} TaskBTGT; // size = 0x1C

typedef struct {
    Vec3F pos;
    Vec3F angle;
    u8 type;
    u8 pad19[3];
} TaskCNTG; // size = 0x1C

typedef struct {
    Vec3F unk0;
    f32 unkC;
    u8 unk10;
    u8 unk11;
    u8 unk12;
    u8 unk13;
    u8 pad14[4];
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 unk28;
    f32 unk2C;
    f32 unk30;
    f32 unk34;
    f32 unk38;
    f32 unk3C;
    f32 unk40;
    f32 unk44;
    f32 unk48;
    f32 unk4C;
    f32 unk50;
    f32 unk54;
    f32 unk58;
    f32 unk5C;
    f32 unk60;
    f32 unk64;
    f32 unk68;
    f32 unk6C;
    f32 unk70;
    f32 unk74;
    f32 unk78;
    f32 unk7C;
    f32 unk80;
    f32 unk84;
    f32 unk88;
    f32 unk8C;
    f32 unk90;
    f32 unk94;
    f32 unk98;
    f32 unk9C;
    f32 unkA0;
    f32 unkA4;
    f32 unkA8;
} TaskFALC; // size = 0xAC

typedef struct {
    s32 unk0;
    Vec3F pos;
    s32 unk10;
    f32 scale;
    f32 height;
    u8 pad1C[4];
} TaskHOPD; // size = 0x20

typedef struct {
    Vec3F pos;
    Vec3F rot;
    s32 unk18;
    u8 type;
    u8 points;
    u8 pad1E[2];
    f32 fuelAdded;
    u8 nextHpadCount;
    u8 pad25[3];
    s32 nextHpads[5];
    u8 active;
    u8 pad3D[3];
} TaskHPAD; // size = 0x40

typedef struct {
    Vec3F pos;
    Vec3F unkC;
    u8 pad18[4];
    Vec3F unk1C;
    u8 pad28[4];
    u8 type;
    u8 pad2D[3];
} TaskLPAD; // size = 0x30

typedef struct {
    Vec3F posUL; // upper-left position
    Vec3F posLR; // lower-right position
    u8 pad10[0x4];
    u8 validStrip;
    u8 pad1D[3];
    f32 landingAlignment;
} TaskLSTP; // size = 0x24

typedef struct {
    Vec3F unk0;
    Vec3F unkC;
    Vec3F unk18;
    Vec3F unk24;
    Vec3F unk30;
    s32 unk3C;
    Vec3F unk40;
    f32 unk4C;
    u8 unk50;
    u8 unk51;
    u8 pad52[2];
} TaskLWIN; // size = 0x54

typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 unkC;
} TaskOBSV; // size = 0x10

typedef struct {
    s32 unk0;
    s32 unk4;
    f32 unk8;
    f32 unkC;
    f32 unk10;
} TaskPHTS; // size = 0x14

typedef struct {
    Vec3F pos;
    Vec3F angle;
    s32 unk18;
    u8 unk1C;
    u8 childCount;
    u8 pad1E[2];
    s32 childRings[5];
    u8 timedChildRingCount;
    u8 pad35[3];
    s32 timedChildRings[5];
    u8 points;
    u8 untimed;
    u8 pad4E[2];
    f32 timeRingDuration;
    u8 size;
    u8 isActive;
    u8 pad56[2];
    f32 rotRate0;
    f32 translation;
    u8 rotAxis0;
    u8 pad61[3];
    f32 rotRateTimedOut1;
    f32 rotRateTiming1;
    f32 rotRate1;
    u8 rotAxis1;
    u8 ringType;
    u8 ringSubtype;
    u8 unk73; // copied, unused
    char name[0x10];
} TaskRNGS; // size = 0x84

typedef struct {
    u8 pad0[0x4C];
} TaskSDFM; // size = 0x4C

typedef struct {
    Vec3F pos;
    Vec3F rot;
    u8 targetType;
    u8 unk19;
    u8 pad1A[2];
    s32 pad1C;
} TaskTARG; // size = 0x20

typedef struct {
    Vec3F pos;
    f32 scale;
    f32 height;
    s32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
} TaskTHER; // size = 0x28

typedef struct {
    Vec3F pos;
    Vec3F angle;
    u8 pad18[4];
    Vec3F unk1C;
    u8 unk28;
    u8 pad29[3];
    f32 unk2C;
} TaskTPAD; // size = 0x30

typedef struct {
    s32 count;
    Vec2F unk4[8]; // how large is this?
    s32 pad44;
    s32 pad48;
    s32 pad4C;
    s32 pad50;
} Unk803599D0; // size = 0x54

typedef struct {
    // keeping these as single members instead of array because it's easier to read
    Unk803599D0 unk0;
    Unk803599D0 unk54;
    Unk803599D0 unkA8;
    Unk803599D0 unkFC;
    Unk803599D0 unk150;
    // maybe more Unk803599D0 here
    u8 pad1A4[0x39C-0x1A4];
    s16 unk39C[4];
    f32 unk3A4[4];
    s32 unk3B4;
    s32 unk3B8;
    f32 unk3BC;
    f32 unk3C0;
    f32 unk3C4;
    f32 unk3C8;
} Unk80345C80;

typedef struct {
    Vec3F unk0;
    f32 unkC;
    f32 unk10;
    f32 unk14;
    f32 unk18;
} TaskObjects_Unk10; // size = 0x1C

typedef struct {
    Vec3F unk0;
    Vec3F unkC;
} TaskObjUnk2C; // size = 0x18

typedef struct {
    struct {
        u8 classNum;
        u8 vehNum;
        u8 testNum;
        u8 mapId;
        u8 unk4;
        u8 unk5[0x3];
        u8 unk8[4];
        f32 unkC;
        TaskObjects_Unk10 unk10;
        TaskObjUnk2C unk2C;
        f32 unk44;
        Unk80345C80 unk48;
        u8 unk414[4];
        f32 unk418;
        u8 countTHER;
        u8 countLWIN;
        u8 countTPAD;
        u8 countLPAD;
        u8 countLSTP;
        u8 countRNGS;
        u8 countBALS;
        u8 countTARG;
        u8 countHPAD;
        u8 countBTGT;
        u8 countPHTS;
        u8 countFALC;
        u8 countSDFM;
        u8 countCNTG;
        u8 countHOPD;
        u8 countOBSV;
    } comm;
    void* dataNAME;
    void* dataINFO;
    void* dataJPTX;
    TaskTHER* dataTHER;
    TaskLWIN* dataLWIN;
    TaskTPAD* dataTPAD;
    TaskLPAD* dataLPAD;
    TaskLSTP* dataLSTP;
    TaskRNGS* dataRNGS;
    TaskBALS* dataBALS;
    TaskTARG* dataTARG;
    TaskHPAD* dataHPAD;
    TaskBTGT* dataBTGT;
    TaskPHTS* dataPHTS;
    TaskFALC* dataFALC;
    TaskSDFM* dataSDFM;
    TaskCNTG* dataCNTG;
    TaskHOPD* dataHOPD;
    TaskOBSV* dataOBSV;
} TaskObjects;

extern u8 D_803507A0; // flag used in map3d for rings and targets

void taskInit(void);
s32 taskIsValidIndex(s32 classIdx, s32 testIdx, s32 vehicle);
s32 taskLoadNames(s32 classIdx, s32 testIdx, s32 vehicle, char*, char*);
s32 taskGetTestCount(s32 classIdx, s32 vehicle);
s32 taskInitTest(s32 classIdx, s32 vehicle, s32 testIdx, u16* map, u16* terraId, u16*);
void taskLoad(void);
void taskDeinitLevel(void);
void taskDeinit(void);
s32 taskFrameUpdate(Mtx4F*, f32);
s32 task_803456D8(Mtx4F*);
void taskUpdateState(void);
s32 taskGetTHER(TaskTHER** data);
s32 taskGetLWIN(TaskLWIN** data);
TaskObjects_Unk10* taskGet_80345AEC(void);
s32 taskGetTPAD(TaskTPAD** data);
s32 taskGetCNTG(TaskCNTG** data);
s32 taskGetOBSV(TaskOBSV** data);
s32 taskGetLPAD(TaskLPAD** data);
s32 taskGetLSTP(TaskLSTP** data);
s32 taskGetRNGS(TaskRNGS** data);
s32 taskGetBALS(TaskBALS** data);
s32 taskGetTARG(TaskTARG** data);
s32 taskGetHPAD(TaskHPAD** data);
s32 taskGetBTGT(TaskBTGT** data);
s32 taskGetPHTS(TaskPHTS** data);
s32 taskGetFALC(TaskFALC** data);
Unk80345C80* taskGet_80345C80(void);
TaskObjUnk2C* taskGet_80345C90(void);
f32 taskGet_80345CA0(void);
u8* taskGet_80345CB0(void);
void taskGet_80345CC0(f32*, f32*);
TaskObjects* taskLoadCommObj(u32 taskIdx);
void taskBirdmanPad(u16 mapIdx);
void taskGetClsVehTest(u16* classIdx, u16* vehIdx, u16* testIdx);
u8 taskGet_80346364(void);
s32 task_80346370(s32 terra);
u8 taskGet_80346468(void);
s32 taskGetHOPD(TaskHOPD** data);

#endif // APP_TASK_H
