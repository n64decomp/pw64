#include "common.h"
#include <uv_filesystem.h>
#include <uv_graphics.h>
#include <uv_memory.h>
#include <uv_vector.h>
#include "code_9A960.h"
#include "demo_attitude.h"
#include "environment.h"
#include "fire_effects.h"
#include "game.h"

typedef struct {
    Vec3F pos;
    f32 rotZ;
    f32 rotX;
    f32 rotY;
} PPOS;

typedef struct {
    s32 duration;
    s32 pposCount;
} PHDR;

typedef struct Unk8034F970 {
    struct Unk8034F970* unk0;
    PPOS* unk4;
} Unk8034F970;

// .data
STATIC_DATA Unk8034F970* D_8034F970 = NULL;

// .bss
STATIC_DATA s32 D_8036D320;
STATIC_DATA s32 D_8036D324;
STATIC_DATA f32 D_8036D328;
STATIC_DATA f32 D_8036D32C;
STATIC_DATA s32 sDemoUserFileIdx;

void demoAttInit(s32 userFileIdx) {
    sDemoUserFileIdx = userFileIdx;
    demoAttLoadPos();
}

void demoAttLoadPos(void) {
    s32 gotHdr;
    s32 sp60;
    u32 size;
    s32 idx;
    void* data;
    u32 tag;
    PHDR* phdr;

    idx = 0;
    gotHdr = 0;
    D_8036D328 = -1.0f;
    sp60 = uvFileReadHeader((s32)uvUserFileRead(sDemoUserFileIdx, MEM_ROM_OFFSET));
    D_8036D320 = 0;
    D_8036D324 = 0;

    while ((tag = uvFileReadBlock(sp60, &size, &data, 1)) != 0) {
        switch (tag) {
        case 'PHDR': // 0x50484452
            phdr = (PHDR*)data;
            D_8036D320 = phdr->duration;
            D_8036D324 = phdr->pposCount;
            D_8034F970 = (Unk8034F970*)_uvMemAllocAlign8(D_8036D324 * sizeof(Unk8034F970));
            if (D_8034F970 == NULL) {
                return;
            }
            D_8034F970->unk4 = (PPOS*)_uvMemAllocAlign8(D_8036D324 * sizeof(PPOS));
            if (D_8034F970->unk4 == 0) {
                return;
            }
            D_8034F970->unk0 = D_8034F970;
            gotHdr = 1;
            break;
        case 'PPOS': // 0x50504F53
            if (gotHdr == 0) {
                _uvDebugPrintf("Didn't get header yet\n");
            } else if (idx < D_8036D324) {
                _uvMediaCopy(&D_8034F970->unk4[idx], data, size);
                idx++;
            }
            break;
        }
    }
    if (idx < D_8036D324) {
        D_8036D324 = idx;
    }
    uvFile_80223F30(sp60);
}

void demoAttGetMtxRT(Mtx4F* mat, f32* tx, f32* ty, f32* tz, f32* rz, f32* rx, f32* ry) {
    func_80313570(mat, tx, ty, tz, rz, rx, ry);
    *rx = -*rx;
}

void demoAttPrintRecord(void) {
    demoAttPrintRT(&D_80362690->unkC[D_80362690->unk9C].unk70->unk108);
}

f32 demoPositionLerp(f32 a, f32 b, f32 t) {
    return ((b - a) * t) + a;
}

f32 demoAngleLerp(f32 a, f32 b, f32 t) {
    f32 diff;

    diff = a - b;
    if (diff > 3.1415927f) {               // PI or DEG_TO_RAD(180)
        b = (f32)(b + 6.2831854820251465); // 2*PI or DEG_TO_RAD(360)
    } else if (diff < -3.1415927f) {       // PI or DEG_TO_RAD(180)
        a = (f32)(a + 6.2831854820251465); // 2*PI or DEG_TO_RAD(360)
    }
    return ((b - a) * t) + a;
}

void demoAttUpdate(f32 curTime, Mtx4F* mat) {
    s32 pad0;
    s32 pad1;
    f32 posX;
    f32 posY;
    f32 posZ;
    f32 rotZ;
    f32 rotX;
    f32 endTime;
    f32 timeStep;
    f32 idxf32;
    f32 rotY;
    f32 time;
    s32 idx;

    if (D_8036D324 != 0) {
        if ((D_8034F970 == NULL) || (D_8034F970 != D_8034F970->unk0)) {
            demoAttLoadPos();
        }
        if ((f64)D_8036D328 == -1.0) {
            D_8036D328 = D_8034F850;
            D_8036D32C = (f32)D_8036D324 / (f32)D_8036D320;
        }
        endTime = (f32)D_8036D324;
        time = ((curTime - D_8036D328) / D_8036D32C) * endTime;
        if ((f64)time < 0.0) {
            time = 0.0f;
        } else if (endTime <= time) {
            time = (f32)(D_8036D324 - 1);
        }
        idx = (s32)time;
        idxf32 = (f32)idx;
        if ((time == idxf32) || ((idx + 1) == D_8036D324)) {
            posX = D_8034F970->unk4[idx].pos.x;
            posY = D_8034F970->unk4[idx].pos.y;
            posZ = D_8034F970->unk4[idx].pos.z;
            rotZ = D_8034F970->unk4[idx].rotZ;
            rotX = D_8034F970->unk4[idx].rotX;
            rotY = D_8034F970->unk4[idx].rotY;
        } else {
            timeStep = time - idxf32;
            posX = demoPositionLerp(D_8034F970->unk4[idx].pos.x, D_8034F970->unk4[idx + 1].pos.x, timeStep);
            posY = demoPositionLerp(D_8034F970->unk4[idx].pos.y, D_8034F970->unk4[idx + 1].pos.y, timeStep);
            posZ = demoPositionLerp(D_8034F970->unk4[idx].pos.z, D_8034F970->unk4[idx + 1].pos.z, timeStep);
            rotZ = demoAngleLerp(D_8034F970->unk4[idx].rotZ, D_8034F970->unk4[idx + 1].rotZ, timeStep);
            rotX = demoAngleLerp(D_8034F970->unk4[idx].rotX, D_8034F970->unk4[idx + 1].rotX, timeStep);
            rotY = demoAngleLerp(D_8034F970->unk4[idx].rotY, D_8034F970->unk4[idx + 1].rotY, timeStep);
        }
        func_80313640(posX, posY, posZ, rotZ, rotX, rotY, mat);
    }
}

void demoAtt_80320FBC(void) {
    Camera* camera;
    camera = D_80362690->unkC[D_80362690->unk9C].unk70;
    demoAttUpdate(D_8034F850, &camera->unk108);
    uvChan_80204B34(D_80362690->unkC[D_80362690->unk9C].unk70->unk22C, &camera->unk108);
    env_802E2060();
    fireFxUpdate();
}

void demoAttPrintRT(Mtx4F* mat) {
    f32 tx;
    f32 ty;
    f32 tz;
    f32 rz;
    f32 rx;
    f32 ry;

    demoAttGetMtxRT(mat, &tx, &ty, &tz, &rz, &rx, &ry);
    _uvDebugPrintf("precord %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n", D_8034F850, tx, ty, tz, rz, rx, ry);
}
