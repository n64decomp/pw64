#include <PR/os_cont.h>
#include <PR/os_eeprom.h>
#include <PR/os_internal.h>
#include <PR/os_system.h>
#include <PR/os_host.h>
#include <uv_audio.h>
#include <uv_chan.h>
#include <uv_clocks.h>
#include <uv_controller.h>
#include <uv_dobj.h>
#include <uv_environment.h>
#include <uv_fx.h>
#include <uv_geometry.h>
#include <uv_graphics.h>
#include <uv_matrix.h>
#include <uv_memory.h>
#include <uv_sched.h>
#include <uv_sprite.h>
#include <uv_system.h>
#include <uv_texture.h>
#include <macros.h>
#include <segment_symbols.h>

u8 gRenderThreadStack[0x2000];
u8 gKernelThreadStack[0x2000];
u8 gAppThreadStack[0x4000];
UNUSED s32 D_802C1C90;
uvLevelData gLevelData;
s32 D_802C32A4;
OSIoMesg gPiDmaBlockReq;
OSMesg gPiDmaBuf[1];
OSMesgQueue gPiDmaQ;
OSMesg gSiContBuf[3];
OSMesgQueue gSiContQ;
OSMesgQueue D_802C3300;
OSMesg D_802C3318[1];
s32 D_802C331C;
OSMesg gPiMgrCmdBuf[8];
OSMesgQueue gPiMgrCmdQ;
OSContStatus gSiContStatus[MAXCONTROLLERS];
UNUSED u8 D_802C3360[4];
OSContPad gControllerPads[6];
u8 gSiContPattern;
OSThread gAppThread;
OSThread gRenderThread;
OSThread gKernelThread;
OSSched gSchedInst;
UNUSED u8 D_802C3B30[0x20];
OSMesgQueue D_802C3B50;
OSMesg D_802C3B68[10];
OSMesgQueue D_802C3B90;
u8 gSchedStack[0x2000];
OSScClient gSchedClient;
s32 gEepromFound;
s32 gControllerPattern;

// where is this ultralib piint.h header?
s32 osPiRawReadIo(u32 devAddr, u32* data);

void Thread_App(void* arg);
void Thread_Kernel(void* arg);
void Thread_Render(void* arg);
void app_entrypoint(s32);
void func_8022E558(void);

void func_8020F9F4(void);

extern s32 gGfxSyncNeeded;
extern s32 gNmiAsserted;

s32 uvFileWrite(u8* dst, u32 offs, u32 nbytes) {
    if (gEepromFound == 0) {
        _uvDebugPrintf("uvFileWrite: no eeprom detected\n");
        return 0;
    }
    if (offs & 0x7) {
        _uvDebugPrintf("uvFileWrite: offs needs to be a multiple of 8\n");
        return 0;
    }
    if (nbytes & 0x7) {
        _uvDebugPrintf("uvFileWrite: nbytes needs to be a multiple of 8\n");
        return 0;
    }
    if ((s32)(offs + nbytes) > 0x208) {
        _uvDebugPrintf("uvFileWrite: offs [%d] + nbytes [%d]  > %d bytes\n", offs, nbytes, 0x208);
        return 0;
    }
    offs = (s32)offs / 8;
    if (osEepromLongWrite(&gSiContQ, offs, dst, nbytes) != 0) {
        _uvDebugPrintf("uvFileWrite: write error\n");
        return 0;
    }
    return nbytes;
}

s32 uvFileRead(void* dst, u32 offs, u32 nbytes) {
    if (gEepromFound == 0) {
        _uvDebugPrintf("uvFileRead: no eeprom detected\n");
        return 0;
    }
    if (offs & 0x7) {
        _uvDebugPrintf("uvFileRead: offs needs to be a multiple of 8\n");
        return 0;
    }
    if (nbytes & 0x7) {
        _uvDebugPrintf("uvFileRead: nbytes needs to be a multiple of 8\n");
        return 0;
    }
    if ((s32)(offs + nbytes) > 0x208) {
        // devs used this `uvFileWrite` string, likely from copy-paste mistake
        _uvDebugPrintf("uvFileWrite: offs [%d] + nbytes [%d]  > %d bytes\n", offs, nbytes, 0x208);
        return 0;
    }
    offs = (s32)offs / 8;
    if (osEepromLongRead(&gSiContQ, offs, dst, nbytes) != 0) {
        _uvDebugPrintf("uvFileRead: read error\n");
        return 0;
    }
    return nbytes;
}

s32 uvSysInit(s32 arg0) {
    _uvDebugPrintf("\n\n");
    _uvDebugPrintf(" ------------------------------------------------------------------\n\n");
    _uvDebugPrintf("\t\t       UltraVision NU64  V1.5\n\n");
    _uvDebugPrintf("   Copyright (C)  1995  by  Paradigm Simulation Incorporated\n");
    _uvDebugPrintf("\t\t    All Rights Reserved Worldwide\n\n");
    _uvDebugPrintf(" ------------------------------------------------------------------\n\n");
    // clang-format off: easier to read on separate lines
    _uvDebugPrintf("kernel : [%7d ] bytes (%7d text, %7d data, %7d bss)\n",
                   SEGMENT_VRAM_SIZE(kernel),
                   SEGMENT_TEXT_SIZE(kernel),
                   SEGMENT_DATA_SIZE(kernel),
                   SEGMENT_BSS_SIZE(kernel));
    _uvDebugPrintf("app    : [%7d ] bytes (%7d text, %7d data, %7d bss)\n",
                   SEGMENT_VRAM_SIZE(app),
                   SEGMENT_TEXT_SIZE(app),
                   SEGMENT_DATA_SIZE(app),
                   SEGMENT_BSS_SIZE(app));
    _uvDebugPrintf("filesys: [%7d ] bytes (0x%08x - 0x%08x)\n",
                   SEGMENT_ROM_SIZE(filesys),
                   SEGMENT_ROM_START(filesys),
                   SEGMENT_ROM_END(filesys));
    // clang-format on
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
    gControllerPattern = 0;
    uvGfxInit();
    uvClkInit();
    uvMemInitBlockHdr();
    uvChanInit();
    uvEnvResetModelView();
    func_8020F9F4();
    uvDobjInit();
    uvControllerInit();

    uvSeqInit();
    uvFxInit();
    uvVtxInit();
    uvSprt_80230130();
    uvSprtInit();
    uvMemInitBlocks();
    uvaManagerInit();
    gEepromFound = osEepromProbe(&gSiContQ);
    if (gEepromFound == 0) {
        _uvDebugPrintf("uvSysInit : no EEPROM found -- read/writes will fail\n");
    }
    return arg0;
}

s32 uvSysUnknownStub(s32 arg0) {
    return arg0;
}

void uvWaitForMesg(char msg_type) {
    switch (msg_type) {
    case UV_MESG_DMA:
        osRecvMesg(&gPiDmaQ, NULL, OS_MESG_BLOCK);
        return;
    case UV_MESG_SCHED:
        osRecvMesg(&D_802C3B50, NULL, OS_MESG_BLOCK);
        return;
    case UV_MESG_GFX:
        osRecvMesg(&D_802C3B90, NULL, OS_MESG_BLOCK);
        gGfxSyncNeeded = FALSE;
        return;
    }
}

void uvSetVideoMode(void) {
#if defined(VERSION_JP)
    osCreateMesgQueue(&D_802C3B90, D_802C3B68, ARRAY_COUNT(D_802C3B68));
    _uvScCreateScheduler(&gSchedInst, gSchedStack + sizeof(gSchedStack), OS_PRIORITY_APPMAX, OS_VI_NTSC_LAN1, 1);
    _uvScAddClient(&gSchedInst, &gSchedClient, &D_802C3B50);
#else // VERSION_US
    s32 viMode;

    osCreateMesgQueue(&D_802C3B90, D_802C3B68, ARRAY_COUNT(D_802C3B68));
    // clang-format off: debug statements need to be on same line for matching
    switch (osTvType) {
    case OS_TV_PAL:
        viMode = OS_VI_PAL_LAN1; _uvDebugPrintf("PAL video mode\n");
        break;
    case OS_TV_NTSC:
        viMode = OS_VI_NTSC_LAN1; _uvDebugPrintf("NTSC video mode\n");
        break;
    default:
        viMode = OS_VI_PAL_LAN1; _uvDebugPrintf("PAL video mode\n");
        break;
    }
    // clang-format on
    _uvScCreateScheduler(&gSchedInst, gSchedStack + sizeof(gSchedStack), OS_PRIORITY_APPMAX, viMode, 1);
    _uvScAddClient(&gSchedInst, &gSchedClient, &D_802C3B50);
#endif
}

void bootproc(void* arg0) {
    u32 devAddr; // var_s1
    s32* var_v0;
    s32 pad;
    u8 sp3C[0x40];
    s32 i;

    for (var_v0 = (s32*)0x800DA800; var_v0 < (s32*)0x80100000; var_v0++) {
        *var_v0 = 0;
    }

    for (var_v0 = (s32*)0x80100000; var_v0 != (s32*)0x80125800; var_v0++) {
        *var_v0 = 0;
    }

    osInitialize();
    devAddr = 0xFFB000;
    for (i = 0; i < ARRAY_COUNT(sp3C); i += 4) {
        osPiRawReadIo(devAddr, (u32*)&sp3C[i]);
        devAddr += 4;
    }

    if (sp3C[0] == 0x2D) {
        if (sp3C[1] == 0x64) {
            D_802C32A4 = 1;
        } else if (sp3C[1] == 0x7A) {
            uvMemClearRegions();
        }
    }
    osCreateThread(&gKernelThread, 1, Thread_Kernel, arg0, gKernelThreadStack + sizeof(gKernelThreadStack), 0xC);
    osStartThread(&gKernelThread);
}

void func_8022E558(void) {
    s16 temp_s3;
    s16 temp_s4;
    s16 spBA;
    u8 var_fp;
    u8 var_s6;
    u8 var_s7;
    Mtx4F sp74;

    uvGfxStatePush();
    uvGfxSetFlags(0xFFF);
    uvGfxClearFlags(GFX_STATE_XLU | GFX_STATE_ZBUFFER | GFX_STATE_CULL_BACK);
    uvMat4SetOrtho(&sp74, 0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT);
    uvGfxMtxProjPushF(&sp74);
    uvMat4SetIdentity(&sp74);
    uvGfxPushMtxUnk(&sp74);
    uvVtxBeginPoly();
    uvVtx(0xF, 0xF, 0, 0, 0, 0, 0x80, 0, 0xFF);
    uvVtx(0x50, 0xF, 0, 0, 0, 0, 0x80, 0, 0xFF);
    uvVtx(0x50, 0x8C, 0, 0, 0, 0, 0x80, 0, 0xFF);
    uvVtx(0xF, 0x8C, 0, 0, 0, 0, 0x80, 0, 0xFF);
    uvVtxEndPoly();

    for (spBA = 0; spBA < 32; spBA++) {
        if (D_802C331C & (1 << (0x1F - spBA))) {
            var_s6 = 0xFF;
            var_s7 = 0xFF;
            var_fp = 0xFF;
        } else {
            var_s6 = 0;
            var_s7 = 0;
            var_fp = 0;
        }
        uvVtxBeginPoly();
        temp_s3 = ((spBA % 4) * 15) + 20;
        temp_s4 = (((0x1F - spBA) / 4) * 15) + 20;
        uvVtx(temp_s3, temp_s4, 0, 0, 0, var_fp, var_s7, var_s6, 0xFF);
        uvVtx(temp_s3 + 10, temp_s4, 0, 0, 0, var_fp, var_s7, var_s6, 0xFF);
        uvVtx(temp_s3 + 10, temp_s4 + 10, 0, 0, 0, var_fp, var_s7, var_s6, 0xFF);
        uvVtx(temp_s3, temp_s4 + 10, 0, 0, 0, var_fp, var_s7, var_s6, 0xFF);
        uvVtxEndPoly();
    }

    uvGfxMtxViewPop();
    uvGfxStatePop();
}

void Thread_Render(void* arg) {
    void* sp1C;

    sp1C = NULL;
    osCreateMesgQueue(&D_802C3300, D_802C3318, 1);
    osSetEventMesg(OS_EVENT_FAULT, &D_802C3300, (void*)0x10);
    osRecvMesg(&D_802C3300, &sp1C, OS_MESG_BLOCK);
    D_802C331C = __osGetCurrFaultedThread()->context.pc;
    while (1) {
        uvGfxBegin();
        func_8022E558();
        uvGfxEnd();
    }
}

void Thread_App(void* arg) {
    _uvMediaCopy(app_TEXT_START, app_ROM_START, app_ROM_END - app_ROM_START);
    uvMemSet(app_BSS_START, 0, app_BSS_END - app_BSS_START);
    app_entrypoint(arg);
}

void Thread_Kernel(void* arg) {
    osCreatePiManager(OS_PRIORITY_PIMGR, &gPiMgrCmdQ, gPiMgrCmdBuf, 8);
    osCreateMesgQueue(&gPiDmaQ, gPiDmaBuf, 1);
    osCreateThread(&gRenderThread, 0, Thread_Render, NULL, gRenderThreadStack + sizeof(gRenderThreadStack), 0xFA);
    osStartThread(&gRenderThread);
    uvSetVideoMode();
    osCreateThread(&gAppThread, 6, Thread_App, arg, gAppThreadStack + sizeof(gAppThreadStack), 0xA);
    if (D_802C32A4 == 0) {
        osStartThread(&gAppThread);
    }
    osSetThreadPri(NULL, 0);
    while (1) { }
}

u8 uvContMesgInit(void) {
    OSMesg mesgBuf[2];
    OSMesgQueue mq;
    OSTimer timer;
    OSMesg dummy;
    u8 temp;

    osCreateMesgQueue(&mq, mesgBuf, ARRAY_COUNT(mesgBuf));
    osSetTimer(&timer, 0x1388, 0, &mq, mesgBuf);
    osRecvMesg(&mq, &dummy, OS_MESG_BLOCK);
    osCreateMesgQueue(&gSiContQ, gSiContBuf, ARRAY_COUNT(gSiContBuf));
    osSetEventMesg(OS_EVENT_SI, &gSiContQ, (void*)0x33333333);
    osContInit(&gSiContQ, &gSiContPattern, gSiContStatus);
    temp = gSiContPattern;
    return temp;
}

s32 uvReadController(ControllerInfo* contInfo, s32 contIdx) {
    void* sp34;
    OSContPad* contPad;
    s32 status;
    f32 correctedY;
    f32 correctedX;
    s32 prevButton;
    s8 stickX;
    s8 stickY;

    if (gNmiAsserted != FALSE) {
        return 0;
    }

    status = osContStartReadData(&gSiContQ);
    if (status & ~CONT_OVERRUN_ERROR) {
        contInfo->stickAxes[0] = 0.0f;
        contInfo->stickAxes[1] = 0.0f;
        contInfo->prevButton = contInfo->button;
        contInfo->button = 0;
        return 0;
    }

    osRecvMesg(&gSiContQ, &sp34, OS_MESG_BLOCK);
    osContGetReadData(&gControllerPads[0]);
    contPad = &gControllerPads[contIdx];
    stickX = contPad->stick_x;
    stickY = contPad->stick_y;

    if (stickX < 0) {
        if (stickX < -68) {
            stickX = -68;
        } else if (stickX > -7) {
            stickX = -7;
        }
        correctedX = (f32)(stickX + 7) / 61.0f;
    } else {
        if (stickX > 68) {
            stickX = 68;
        } else if (stickX < 7) {
            stickX = 7;
        }
        correctedX = (f32)(stickX - 7) / 61.0f;
    }

    if (stickY < 0) {
        if (stickY < -70) {
            stickY = -70;
        } else if (stickY > -7) {
            stickY = -7;
        }
        correctedY = (f32)(stickY + 7) / 63.0f;
    } else {
        if (stickY > 70) {
            stickY = 70;
        } else if (stickY < 7) {
            stickY = 7;
        }
        correctedY = (f32)(stickY - 7) / 63.0f;
    }
    contInfo->stickAxes[0] = correctedX;
    contInfo->stickAxes[1] = correctedY;
    contInfo->prevButton = (s32)contInfo->button;
    contInfo->button = (s32)contPad->button;
    return 1;
}

void _uvDebugPrintf(char* fmt, ...) {
}

void _uvDMA(void* vAddr, u32 devAddr, u32 nbytes) {
    s32 dest = vAddr;
    if (gNmiAsserted == FALSE) {
        if (dest % 8) {
            _uvDebugPrintf("_uvDMA: RAM address not 8 byte aligned 0x%x\n", dest);
            return;
        }
        if ((s32)devAddr % 2) {
            _uvDebugPrintf("_uvDMA: ROM address not 2 byte aligned 0x%x\n", devAddr);
            return;
        }
        if ((u32)osMemSize < nbytes) {
            _uvDebugPrintf("_uvDMA: nbytes invalid %d\n", (s32)nbytes);
            return;
        }
        if (nbytes & 1) {
            nbytes = (nbytes + 1) & ~1;
        }
        osWritebackDCache((void*)dest, (s32)nbytes);
        osPiStartDma((OSIoMesg*)&gPiDmaBlockReq, 0, 0, devAddr, (void*)dest, nbytes, &gPiDmaQ);
        osInvalDCache((void*)dest, (s32)nbytes);
        uvWaitForMesg(UV_MESG_DMA);
    }
}

void _uvAssertMsg(const char* expr, const char* filename, int line) {
    _uvDebugPrintf("%s:%d  %s\n", filename, line, expr);
}
