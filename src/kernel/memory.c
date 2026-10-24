#include <uv_filesystem.h>
#include <uv_graphics.h>
#include <uv_memory.h>
#include <uv_graphics.h>
#include <uv_sprite.h>
#include <segment_symbols.h>
#include <macros.h>
#include <PR/os_exception.h>
#include <PR/os_pi.h>
#include <PR/os_system.h>

typedef struct {
    u32 start;
    u32 end;
} Struct802B8830;

typedef struct {
    u32 unk0;
    u32 unk4;
} Struct802B8880;

typedef struct {
    float unk0;
    u16 unk4;
} Struct802B53C8;

s32 D_802B8820;
s32 D_802B8824;
s32 D_802B8828;
s32 D_802B882C;
Struct802B8830 D_802B8830[10];
Struct802B8880 D_802B8880[20];
// flags to determine if any of 3 scratch regions are in use
s32 D_802B8920[3];
u32 D_802B892C;
u32 D_802B8930;
u32 D_802B8934;

extern u8 D_803805E0;

void uvMemInitBlocks(void) {
    D_802B8830[0].start = 0x800DA800;
    D_802B8830[0].end = 0x80100000;
    D_802B8830[1].start = 0x80100000;
    D_802B8830[1].end = 0x80125800;
    D_802B8830[2].start = 0x803DA800;
    D_802B8830[2].end = 0x80400000;
    D_802B8830[3].start = 0x80000400;
    D_802B8830[3].end = 0x800417DC;
    D_802B8830[4].start = 0x80200000;
    D_802B8830[4].end = &D_803805E0;
    D_802B8830[5].start = 0x80000000;
    D_802B8830[5].end = 0x80000400;
    D_802B8820 = 6;
    D_802B8824 = 0;
    D_802B8920[0] = 0;
    D_802B8920[1] = 0;
    D_802B8920[2] = 0;
    uvMemScanBlocks();
}

void uvMemClearRegions(void) {
    u32* a;

    for (a = (u32*)0x800417DC; a < (u32*)0x800DA800; a++) {
        *a = 0;
    }

    // the end of this segment is 802000A0 which coincides with kernel_TEXT_START
    for (a = (u32*)0x80125800; a < (u32*)&kernel_TEXT_START; a++) {
        *a = 0;
    }

    for (a = (u32*)&D_803805E0; a < (u32*)0x803DA800; a++) {
        *a = 0;
    }
}

void uvMemScanBlocks(void) {
    s32 j;
    s32 i;

    for (i = 0; i < D_802B8820; i++) {
        for (j = 0; j < D_802B8820; j++) {
            if (i == j) {
                continue;
            }
            if ((D_802B8830[j].start < D_802B8830[i].end) && (D_802B8830[i].start < D_802B8830[j].end)) {
                _uvDebugPrintf("uvSysInit: memory overlap with blocks %d and %d\n", i, j);
                _uvDebugPrintf("           [ 0x%x , 0x%x ] and [ 0x%x , 0x%x ]\n", D_802B8830[i].start, D_802B8830[i].end, D_802B8830[j].start,
                               D_802B8830[j].end);
                break;
            }
        }
    }
}

s32 func_8022A5F4(u32 startAddr, u32 length) {
    s32 i;
    u32 endAddr;

    endAddr = startAddr + length;

    for (i = 0; i < D_802B8820; i++) {
        if ((endAddr >= D_802B8830[i].start) && (endAddr < (u32)D_802B8830[i].end)) {
            return 0;
        }
        if ((startAddr >= D_802B8830[i].start) && (startAddr < (u32)D_802B8830[i].end)) {
            return 0;
        }
    }

    return 1;
}

s32 _uvJumpHeap(u32* arg0) {
    s32 iter;
    s32 retVal;
    Struct802B8830* block;

    retVal = TRUE;
    for (iter = 0; iter < D_802B8820; iter++) {
        block = &D_802B8830[iter];
        if ((*arg0 >= block->start) && (*arg0 < block->end)) {
            iter = 0;
            retVal = FALSE;
            *arg0 = block->end;
            if (D_802B8934 != 0) {
                _uvDebugPrintf("_uvJumpHeap: jumping to txt img data 0x%x\n", *arg0);
            }
        }
    }
    return retVal;
}

void _uvMediaCopy(void* vAddr, void* devAddr, u32 nbytes) {
    s32 i;
    u8* alignCeil;
    s32 alignDiff;
    u8 buf[8];
    u8* dst;
    u8* src;

    dst = vAddr;
    src = devAddr;
    if ((u32)src & 0x80000000) {
        for (i = 0; (u32)i < nbytes; i++) {
            dst[i] = src[i];
        }
        return;
    }

    while (nbytes > 0x1000) {
        _uvMediaCopy(dst, src, 0x1000);
        nbytes -= 0x1000;
        dst += 0x1000;
        src += 0x1000;
    }

    if (nbytes != 0) {
        if (((s32)src | (s32)dst | nbytes) & 1) {
            _uvDebugPrintf("Media copy src and dst && nbytes must be even\n");
            // it appears the devs intended to cause a fault by storing to an unaligned address,
            // but IDO outsmarted them and broke this into two `sb` instructions
            *(u16*)(1) = 0;
            return;
        }
        if ((s32)dst & 7) {
            alignCeil = ALIGN8((s32)dst);
            alignDiff = alignCeil - dst;
            if ((nbytes != (u32)alignDiff) != 0) {
                _uvDMA(alignCeil, src + alignDiff, nbytes - alignDiff);
            }
            osPiReadIo(src, (u32*)&buf[0]);
            osPiReadIo(src + 4, (u32*)&buf[4]);
            for (i = 0; i < alignDiff && (u32)i < nbytes; i++) {
                dst[i] = buf[i];
            }
        } else {
            _uvDMA(dst, src, nbytes);
        }
    }
}

u64 uvMemRead(void* vAddr, u32 nbytes) {
    u64 out;
    u64 temp1;
    s64 temp2;
    u8* src;

    src = vAddr;
    if ((nbytes != 1) && (nbytes != 2) && (nbytes != 4) && (nbytes != 8)) {
        _uvDebugPrintf("uvMemRead was called with an invalid size\n");
        out = 0;
    } else if ((u32)src & 0x80000000) {
        out = src[0];
        if (nbytes >= 2U) {
            temp1 = out << 8;
            out = temp1 | src[1];
        }
        if (nbytes >= 3U) {
            temp1 = out << 16;
            out = temp1 | (src[2] << 8) | src[3];
        }
        if (nbytes >= 5U) {
            temp1 = out << 32;
            out = temp1 | (src[4] << 0x18) | (src[5] << 0x10) | (src[6] << 8) | src[7];
        }
    } else {
        _uvMediaCopy(&temp2, src, nbytes);
        out = uvMemRead((u8*)&temp2, nbytes);
    }
    return out;
}

void uvMemSet(void* vAddr, u8 value, u32 nbytes) {
    s32 count = 0;
    u8* dest = vAddr;
    while (count < nbytes) {
        dest[count++] = value;
    }
}

s32 uvMemCmp(u8* lhs, u8* rhs, u32 count) {
    s32 i;

    for (i = 0; (u32)i < count; i++) {
        if (lhs[i] != rhs[i]) {
            if (lhs[i] < rhs[i]) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

s32 _uvMemOverAlloc(u32 size, u32 alignment) {
    u32 alignedStart;

    for (;;) {
        alignedStart = ((D_802B892C + alignment) - 1) & ~(alignment - 1);
        if (D_802B8880[D_802B8828].unk4 >= alignedStart + size) {
            D_802B892C = alignedStart + size;
            D_802B8934 += size;
            return alignedStart;
        }
        D_802B8828++;
        _uvDebugPrintf("_uvMemOverAlloc: using img data block %d/%d (all is ok)\n", D_802B8828 + 1, D_802B8824);
        if (D_802B8828 >= D_802B8824) {
            _uvDebugPrintf("_uvMemOverAlloc: ABSOLUTELY NO IMAGE MEMORY LEFT\n");
            return 0;
        }
        D_802B892C = D_802B8880[D_802B8828].unk0;
    }
}

s32 _uvMemAlloc(u32 size, u32 alignment) {
    u32 alignedStart;

    alignedStart = ((D_802B892C + alignment) - 1) & ~(alignment - 1);
    if (D_802B8934 != 0) {
        return _uvMemOverAlloc(size, alignment);
    }

    if (func_8022A5F4(alignedStart, size) == 0) {
        alignedStart += size;
        _uvJumpHeap(&alignedStart);
    }

    D_802B892C = alignedStart + size;
    if (D_802B892C >= D_802B8930) {
        _uvDebugPrintf("_uvMemAlloc: out of memory allocating texture image data\n");
        D_802B892C = D_802B8880->unk0;
        D_802B8934 += size;
        D_802B8828 = 0;
        return _uvMemOverAlloc(size, alignment);
    }
    return alignedStart;
}

void _uvMemFreeScratch(void* addr) {
    switch ((u32)addr) {
    case 0x803DA800:
        D_802B8920[0] = 0;
        break;
    case 0x800DA800:
        D_802B8920[1] = 0;
        break;
    case 0x80100000:
        D_802B8920[2] = 0;
        break;
    default:
        _uvDebugPrintf("_uvMemFreeScratch: 0x%x not a scratch area\n", addr);
        break;
    }
}

void* _uvMemGetScratch(u32 size) {
    if (size >= SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u16)) {
        _uvDebugPrintf("_uvMemGetScratch: size too big ( %d bytes )\n", size);
    } else {
        // clang-format off:returns need to be on same line to match reordering
        if (D_802B8920[0] == 0) {
            D_802B8920[0] = 1; return 0x803DA800;
        } else if (D_802B8920[1] == 0) {
            D_802B8920[1] = 1; return 0x800DA800;
        } else if (D_802B8920[2] == 0) {
            D_802B8920[2] = 1; return 0x80100000;
        } else {
            _uvDebugPrintf("_uvMemGetScratch: all scratch areas are full\n", size);
        }
        // clang-format on
    }
    return NULL;
}

void _uvMemGetBlocks(u32 arg0, u32 arg1) {
    if (arg0 != arg1) {
        if (D_802B8824 >= 0x14) {
            _uvDebugPrintf("out of open blocks\n");
        } else {
            if (func_8022A5F4(arg0, arg1 - arg0) != 0) {
                D_802B8880[D_802B8824].unk0 = arg0;
                D_802B8880[D_802B8824].unk4 = arg1;
                D_802B8824 += 1;
            }
        }
    }
}

void uvLevelInit(void) {
    s32 temp_v0;
    u32 tag;
    u32 length;
    void* source;

    uvGfxWaitForMesg();
    uvSprtInit();
    D_802B892C = 0x8004181C;
    D_802B8930 = osMemSize + 0x80000000;
    D_802B8824 = 0;
    D_802B8828 = 0;
    D_802B53C0 = NULL;
    D_802B8934 = 0;
    uvMemClearRegions();
    uvMemSet(&gLevelData, 0, sizeof(gLevelData));
    uvMemSet(D_802B6E30, 0, sizeof(D_802B6E30));
    uvSprtResetUnk();
    temp_v0 = uvFileReadHeader(gUVBlockOffsets.UVSY[0]);

    while ((tag = uvFileReadBlock(temp_v0, &length, &source, 0)) != 0) {
        switch (tag) {
        case 'COMM':
            _uvMediaCopy(&gUVBlockCounts, source, length);
            gLevelData.unk1608 = gUVBlockCounts.unk0;
            break;
        default:
            break;
        }
    }
    uvFile_80223F30(temp_v0);
    if (gUVBlockCounts.uvVersion != UV_KERNEL_VERSION) {
        _uvDebugPrintf("uvLevelInit: dbase [ver %d] and kernel [ver %d] out of date\n", gUVBlockCounts.uvVersion, UV_KERNEL_VERSION);
    }
}

s32 _uvMemAllocAlign8(u32 size) {
    return _uvMemAlloc(size, 8);
}
