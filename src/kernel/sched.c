#include <PR/os_vi.h>
#include <uv_clocks.h>
#include <uv_math.h>
#include <uv_sched.h>
#include <uv_debug.h>

#define VIDEO_MSG 666
#define RSP_DONE_MSG 667
#define RDP_DONE_MSG 668
#define PRE_NMI_MSG 669

ScheduleEventLog gSchedCpuEvents[5][30];
ScheduleEventLog gSchedRspEvents[5][30];
s32 D_802B9C00[5];
s32 D_802B9C18[5];
f64 D_802B9C30[5];
OSScTask* D_802B9C58;
OSScTask* D_802B9C60[2];
u8 D_802B9C68;
u8 gSchedRspStatus;
u8 gSchedRdpStatus;
u8 D_802B9C6B;
u8 D_802B9C6C;
u8 D_802B9C6D;
u8 D_802B9C6E;
u8 D_802B9C6F;
u8 D_802B9C70;
s32 D_802B9C74;
s32 gSchedRingIdx;
s32 D_802B9C7C;
s32 gNmiAsserted;
s32 D_802B9C84;
s32 D_802B9C88;

s32 D_8024B260 = 0x2;

void uvSc_8022B0A0(Unk8022B0A0* arg0, Mtx4F* arg1) {
    f32 temp_fa1;
    f32 temp_fv0;
    f32 temp_fv1;
    s32 var_v0;

    temp_fa1 = arg1->m[0][0] + arg1->m[1][1] + arg1->m[2][2];

    if (temp_fa1 >= 0.0f) {
        temp_fv0 = uvSqrtF(temp_fa1 + 1.0f);
        temp_fv1 = 0.5f / temp_fv0;
        arg0->unkC = temp_fv0 * 0.5f;
        arg0->unk0 = (arg1->m[2][1] - arg1->m[1][2]) * temp_fv1;
        arg0->unk4 = (arg1->m[0][2] - arg1->m[2][0]) * temp_fv1;
        arg0->unk8 = (arg1->m[1][0] - arg1->m[0][1]) * temp_fv1;
        return;
    }
    var_v0 = 0;
    if (arg1->m[0][0] < arg1->m[1][1]) {
        var_v0 = 1;
    }
    if (arg1->m[var_v0][var_v0] < arg1->m[2][2]) {
        var_v0 = 2;
    }
    switch (var_v0) {
    case 0:
        temp_fv0 = uvSqrtF((arg1->m[0][0] - (arg1->m[1][1] + arg1->m[2][2])) + 1.0f);
        temp_fv1 = 0.5f / temp_fv0;
        arg0->unk0 = temp_fv0 * 0.5f;
        arg0->unk4 = (arg1->m[0][1] + arg1->m[1][0]) * temp_fv1;
        arg0->unk8 = (arg1->m[2][0] + arg1->m[0][2]) * temp_fv1;
        arg0->unkC = (arg1->m[2][1] - arg1->m[1][2]) * temp_fv1;
        break;
    case 1:
        temp_fv0 = uvSqrtF((arg1->m[1][1] - (arg1->m[2][2] + arg1->m[0][0])) + 1.0f);
        temp_fv1 = 0.5f / temp_fv0;
        arg0->unk4 = temp_fv0 * 0.5f;
        arg0->unk8 = (arg1->m[1][2] + arg1->m[2][1]) * temp_fv1;
        arg0->unk0 = (arg1->m[0][1] + arg1->m[1][0]) * temp_fv1;
        arg0->unkC = (arg1->m[0][2] - arg1->m[2][0]) * temp_fv1;
        break;
    case 2:
        temp_fv0 = uvSqrtF((arg1->m[2][2] - (arg1->m[0][0] + arg1->m[1][1])) + 1.0f);
        temp_fv1 = 0.5f / temp_fv0;
        arg0->unk8 = temp_fv0 * 0.5f;
        arg0->unk0 = (arg1->m[2][0] + arg1->m[0][2]) * temp_fv1;
        arg0->unk4 = (arg1->m[1][2] + arg1->m[2][1]) * temp_fv1;
        arg0->unkC = (arg1->m[1][0] - arg1->m[0][1]) * temp_fv1;
        break;
    }
}

void _uvScDoneGfx(void) {
    OSScTask* scTask = D_802B9C60[D_802B9C6E];

    if (gNmiAsserted != 0) {
        return;
    }

    if (scTask == NULL) {
        _uvDebugPrintf("_uvScDoneGfx -- no gfx task\n");
        _uvScLogIntoRing();
        return;
    }

    if (D_802B9C6C != 0) {
        return;
    }

    if ((gSchedRspStatus == 'g') || (gSchedRdpStatus != 0)) {
        _uvDebugPrintf("_uvScDoneGfx:  rsp [%c]    rdp [%c]\n", gSchedRspStatus, gSchedRdpStatus);
        return;
    }

    uvSc_8022C3C0(FALSE, EVENT_ID_KERNEL_WAKEUP);
    osSendMesg(scTask->msgQ, scTask->msg, 1);
    D_802B9C68 = 1;
    osViSwapBuffer(scTask->framebuffer);
    D_802B9C60[D_802B9C6E] = NULL;
}

void _uvScDoneAud(void) {
    if (D_802B9C58 == NULL) {
        _uvDebugPrintf("_uvScDoneAud: no audio task\n");
        return;
    }
    uvSc_8022C3C0(TRUE, EVENT_ID_AUDIO_DONE);
    if (gNmiAsserted == 0) {
        osSendMesg(D_802B9C58->msgQ, D_802B9C58->msg, 1);
    }
    D_802B9C58 = NULL;
    if (D_802B9C60[D_802B9C6E] != 0) {
        _uvScRunGfx();
    }
}

void _uvScRunAud(void) {
    if (gNmiAsserted != 0) {
        return;
    }

    if (D_802B9C58 == NULL) {
        _uvDebugPrintf("_uvScRunAud -- no audio task\n");
        return;
    }

    gSchedRspStatus = 'a';
    uvSc_8022C3C0(TRUE, EVENT_ID_AUDIO_START);
    osWritebackDCacheAll();
    osSpTaskLoad(&D_802B9C58->list);
    osSpTaskStartGo(&D_802B9C58->list);
}

void _uvScRunGfx(void) {
    OSScTask* scTask;

    scTask = D_802B9C60[D_802B9C6E];
    if (gNmiAsserted != 0 && D_802B9C6C == 0) {
        return;
    }

    if (scTask == NULL) {
        _uvDebugPrintf("_uvScRunGfx -- no gfx task\n");
        return;
    }

    if (gSchedRspStatus == 'g' || (D_802B9C6C == 0 && gSchedRdpStatus == 'g')) {
        return;
    }

    if (osViGetCurrentFramebuffer() != osViGetNextFramebuffer()) {
        return;
    }

    gSchedRspStatus = 'g';
    if (D_802B9C6C == 0) {
        D_802B9C74++;
        gSchedRdpStatus = 'g';
    }

    uvSc_8022C3C0(TRUE, EVENT_ID_GFX_START);
    D_802B9C6C = D_802B9C6B = 0;
    osWritebackDCacheAll();
    osSpTaskLoad(&scTask->list);
    osSpTaskStartGo(&scTask->list);
}

void _uvScDlistRecover(void) {
    _uvDebugPrintf("Recovered from a bad display list\n");

    IO_WRITE(SP_STATUS_REG, 0x2902);
    if (gSchedRspStatus != 0) {
        osSendMesg(&gSchedInst.interruptQ, (OSMesg)RSP_DONE_MSG, OS_MESG_NOBLOCK);
    }
    if (gSchedRdpStatus != 0) {
        osSendMesg(&gSchedInst.interruptQ, (OSMesg)RDP_DONE_MSG, OS_MESG_NOBLOCK);
    }
}

void _uvScCreateScheduler(OSSched* sc, void* stack, s32 priority, u8 mode, u8 numFields) {
    D_802B9C60[1] = 0;
    D_802B9C60[0] = 0;
    D_802B9C58 = 0;
    D_802B9C68 = 0;
    gSchedRspStatus = 0;
    gSchedRdpStatus = 0;
    D_802B9C6B = 0;
    D_802B9C6C = 0;
    D_802B9C6D = 0;
    D_802B9C6E = 0;
    D_802B9C6F = 0;
    D_802B9C70 = 0;
    D_802B9C74 = 0;
    gSchedRingIdx = 0;
    D_802B9C7C = 0;
    gNmiAsserted = 0;
    D_802B9C84 = 1;
    D_802B9C88 = 1;

    if ((sc && sc) != 0) { } // fakematch

    sc->clientList = 0;
    sc->curRSPTask = 0;
    sc->curRDPTask = 0;
    sc->frameCount = 0;
    sc->audioListHead = 0;
    sc->gfxListHead = 0;
    sc->audioListTail = (OSScTask*)&sc->audioListHead;
    sc->gfxListTail = (OSScTask*)&sc->gfxListHead;
    sc->retraceMsg.type = OS_SC_RETRACE_MSG;
    sc->prenmiMsg.type = OS_SC_PRE_NMI_MSG;

    osCreateMesgQueue(&sc->interruptQ, sc->intBuf, OS_SC_MAX_MESGS);
    osCreateMesgQueue(&sc->cmdQ, sc->cmdMsgBuf, OS_SC_MAX_MESGS);
    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[mode]);
    osViSwapBuffer((void*)0x80100000);
    osViBlack(TRUE);
    osSetEventMesg(OS_EVENT_SP, &sc->interruptQ, (OSMesg)RSP_DONE_MSG);
    osSetEventMesg(OS_EVENT_DP, &sc->interruptQ, (OSMesg)RDP_DONE_MSG);
    osSetEventMesg(OS_EVENT_PRENMI, &sc->interruptQ, (OSMesg)PRE_NMI_MSG);
    osViSetEvent(&sc->interruptQ, (OSMesg)VIDEO_MSG, (u32)numFields);

    osCreateThread(&sc->thread, 4, _uvScMain, (void*)sc, stack, priority);
    osStartThread(&sc->thread);

    uvClkReset(UV_CLKID_SCHED);
    uvSc_8022C34C();
}

void _uvScAddClient(OSSched* sc, OSScClient* client, OSMesgQueue* mq) {
    client->msgQ = mq;
    client->next = sc->clientList;
    sc->clientList = (OSScClient*)client;
}

OSMesgQueue* _uvScGetCmdQ(OSSched* s) {
    return &s->cmdQ;
}

void _uvScMain(void* arg0) {
    OSMesg msg;
    msg = NULL;

    while (1) {
        osRecvMesg(&gSchedInst.interruptQ, &msg, OS_MESG_BLOCK);

        switch ((int)msg) {
        case VIDEO_MSG:
            _uvScHandleRetrace();
            break;
        case RSP_DONE_MSG:
            _uvScHandleRSP();
            break;
        case RDP_DONE_MSG:
            _uvScHandleRDP();
            break;
        case PRE_NMI_MSG:
            _uvScHandleNMI();
            break;
        default:
            _uvDebugPrintf("unknown sched interrupt mesg: 0x%x\n", msg);
            break;
        }
    }
}

void _uvScHandleRetrace(void) {
    OSScTask* msg;
    u32 taskType;
    OSScClient* var_s0;

    msg = NULL;
    uvClkUpdate();
    if (gNmiAsserted != 0) {
        return;
    }

    D_802B9C84 = 1;
    if (gSchedRspStatus) {
        D_802B9C70++;
    } else {
        D_802B9C70 = 0;
    }

    if (gSchedRdpStatus) {
        D_802B9C6F++;
    } else {
        D_802B9C6F = 0;
    }

    if (D_802B9C70 >= 51) {
        _uvDebugPrintf("RSP timeout on %c, sending wakeup...\n", gSchedRspStatus);
        _uvScDlistRecover();
        return;
    }

    if ((D_802B9C70 + 50) < D_802B9C6F) {
        D_802B9C6F = 0;
        _uvDebugPrintf("RDP timeout on %c [%d], sending wakeup...\n", gSchedRdpStatus, gSchedRdpStatus);
        _uvScLogIntoRing();
        osSendMesg(&gSchedInst.interruptQ, (OSMesg)RDP_DONE_MSG, OS_MESG_NOBLOCK);
        return;
    }

    if (gSchedRspStatus == 'a') {
        _uvDebugPrintf("_uvScHandleRetrace: audio still busy\n");
        return;
    }

    if (D_802B9C68 != 0) {
        D_802B9C68 = 0;
        uvSc_8022C34C();
        D_802B9C6E ^= 1;
    }

    while (osRecvMesg(&gSchedInst.cmdQ, (OSMesg)&msg, OS_MESG_NOBLOCK) != -1) {
        if (msg == NULL) {
            _uvDebugPrintf("received message with task of 0\n");
            break;
        }

        taskType = msg->list.t.type;
        if (taskType == M_GFXTASK) {
            D_802B9C60[D_802B9C6D] = msg;
            D_802B9C6D ^= 1;
        } else if (taskType == M_AUDTASK) {
            D_802B9C58 = msg;
        } else {
            _uvDebugPrintf("_uvScHandleRetrace: unkown task type 0x%x\n", msg);
        }
    }

    if (D_802B9C58 != NULL) {
        if (gSchedRspStatus == 'g') {
            if (D_802B9C6C != 0) {
                _uvDebugPrintf("_uvScHandleRetrace:  2nd yield\n");
            }
            if (D_802B9C6B != 0) {
                _uvDebugPrintf("_uvScHandleRetrace: gfx task didn't yield\n");
                _uvScDlistRecover();
                return;
            }
            D_802B9C6B = 1;
            uvSc_8022C3C0(TRUE, EVENT_ID_YIELD_REQUEST);
            osSpTaskYield();
        } else {
            _uvScRunAud();
        }
    } else if (D_802B9C60[D_802B9C6E] != NULL) {
        _uvScRunGfx();
    }

    var_s0 = gSchedInst.clientList;
    while (var_s0 != NULL) {
        osSendMesg(var_s0->msgQ, &gSchedInst, 0);
        var_s0 = var_s0->next;
    }
}

void _uvScHandleRSP(void) {
    if (gSchedRspStatus == 0) {
        _uvDebugPrintf("_uvScHandleRSP -- state error, should have been busy\n");
        return;
    }

    D_802B9C70 = 0;
    if (gSchedRspStatus == 'a') {
        gSchedRspStatus = 0;
        _uvScDoneAud();
        return;
    }

    gSchedRspStatus = 0;
    if (D_802B9C6B != 0) {
        if (osSpTaskYielded(&D_802B9C60[D_802B9C6E]->list) != 0) {
            D_802B9C6C = 1;
            uvSc_8022C3C0(TRUE, EVENT_ID_GFX_YIELD);
            if (gNmiAsserted != 0) {
                D_802B9C58 = 0;
                D_802B9C6B = 0;
                _uvScRunGfx();
                return;
            }

        } else {
            uvSc_8022C3C0(TRUE, EVENT_ID_GFX_DONE);
        }

        D_802B9C6B = 0;
        if (D_802B9C58 != 0) {
            _uvScRunAud();
        }
    } else {
        uvSc_8022C3C0(TRUE, EVENT_ID_GFX_DONE);
    }

    if ((gSchedRspStatus != 'g') && (gSchedRdpStatus == 0)) {
        _uvScDoneGfx();
    }
}

void _uvScHandleRDP(void) {
    gSchedRdpStatus = 0;
    D_802B9C6F = 0;
    uvSc_8022C3C0(TRUE, EVENT_ID_GFX_RDP_DONE);
    if ((gSchedRspStatus != 'g') && (D_802B9C6C == 0)) {
        _uvScDoneGfx();
    }
}

void _uvScHandleNMI(void) {
    gNmiAsserted = 1;
    osViBlack(TRUE);
}

void uvSc_8022BEB8(s32 arg0) {
    D_802B9C88 = arg0;
}

void _uvScLogCpuEvent(s32 arg0) {
    s32 i;
    s32 rspStatus;
    f64 temp_fs0;
    s32 rdpStatus;

    temp_fs0 = D_802B9C30[(gSchedRingIdx + 1) % 5];
    if (D_802B9C88 == 0) {
        return;
    }

    _uvDebugPrintf("-------- CPU events ---------\n");

    for (i = 0; i < D_802B9C00[arg0]; i++) {
        _uvDebugPrintf("%d/%d time: %f ", i + 1, D_802B9C00[arg0], gSchedCpuEvents[arg0][i].timestamp - temp_fs0);
        switch (gSchedCpuEvents[arg0][i].eventID) {
        case EVENT_ID_GFX_START:
            _uvDebugPrintf("gfx start\n");
            break;
        case EVENT_ID_GFX_DONE:
            _uvDebugPrintf("gfx done\n");
            break;
        case EVENT_ID_GFX_SCHED:
            _uvDebugPrintf("gfx sched\n");
            break;
        case EVENT_ID_KERNEL_WAKEUP:
            _uvDebugPrintf("kernel wakeup\n");
            break;
        case EVENT_ID_AUDIO_START:
            _uvDebugPrintf("audio start\n");
            break;
        case EVENT_ID_AUDIO_DONE:
            _uvDebugPrintf("audio done\n");
            break;
        case EVENT_ID_AUDIO_SCHED:
            _uvDebugPrintf("audio sched\n");
            break;
        case EVENT_ID_USER1:
            _uvDebugPrintf("user1\n");
            break;
        case EVENT_ID_USER2:
            _uvDebugPrintf("user2\n");
            break;
        case EVENT_ID_USER3:
            _uvDebugPrintf("user3\n");
            break;
        default:
            _uvDebugPrintf("unknown type %d\n", gSchedCpuEvents[arg0][i].eventID);
            break;
        }
    }

    _uvDebugPrintf("---------  RSP events ---------\n");

    for (i = 0; i < D_802B9C18[arg0]; i++) {
        rspStatus = (u8)(gSchedRspEvents[arg0][i].unkC >> 0x18) & 0xFF;
        if (rspStatus == 0) {
            rspStatus = ' ';
        }
        rdpStatus = (u8)(gSchedRspEvents[arg0][i].unkC >> 0x10) & 0xFF;
        if (rdpStatus == 0) {
            rdpStatus = ' ';
        }
        _uvDebugPrintf("%d/%d time: %f  sp: %2c  dp: %2c  st: 0x%x   ", i + 1, D_802B9C18[arg0], gSchedRspEvents[arg0][i].timestamp - temp_fs0, rspStatus,
                       rdpStatus, gSchedRspEvents[arg0][i].unkC & 0xF);

        switch (gSchedRspEvents[arg0][i].eventID) {
        case EVENT_ID_GFX_START:
            _uvDebugPrintf("gfx start\n");
            break;
        case EVENT_ID_GFX_DONE:
            _uvDebugPrintf("gfx rsp done\n");
            break;
        case EVENT_ID_GFX_YIELD:
            _uvDebugPrintf("gfx yield\n");
            break;
        case EVENT_ID_YIELD_REQUEST:
            _uvDebugPrintf("yield request\n");
            break;
        case EVENT_ID_GFX_RDP_DONE:
            _uvDebugPrintf("gfx rdp done\n");
            break;
        case EVENT_ID_AUDIO_START:
            _uvDebugPrintf("audio start\n");
            break;
        case EVENT_ID_AUDIO_DONE:
            _uvDebugPrintf("audio done\n");
            break;
        case EVENT_ID_USER1:
            _uvDebugPrintf("user1\n");
            break;
        case EVENT_ID_USER2:
            _uvDebugPrintf("user2\n");
            break;
        case EVENT_ID_USER3:
            _uvDebugPrintf("user3\n");
            break;
        default:
            _uvDebugPrintf("unknown type %d\n", gSchedRspEvents[arg0][i].eventID);
            break;
        }
    }
}

void _uvScLogIntoRing(void) {
    s32 ring;
    s32 iter;

    _uvDebugPrintf("logging into ring %d\n", gSchedRingIdx);
    for (iter = 0; iter < 5; iter++) {
        ring = (gSchedRingIdx + iter + 1) % 5;
        _uvDebugPrintf("============== ring %d   start time: %f ==============\n", ring, D_802B9C30[ring]);
        _uvScLogCpuEvent(ring);
    }

    D_802B9C84 = 0;
}

void uvSc_8022C34C(void) {
    gSchedRingIdx++;
    if (gSchedRingIdx >= 5) {
        gSchedRingIdx = 0;
    }

    D_802B9C30[gSchedRingIdx] = uvClkGetSec(UV_CLKID_SCHED);
    D_802B9C18[gSchedRingIdx] = 0;
    D_802B9C00[gSchedRingIdx] = 0;
}

void uvSc_8022C3C0(u8 arg0, s32 eventID) {
    ScheduleEventLog* var_v0;
    u32 idx;

    if (D_802B9C88 == 0) {
        return;
    }

    idx = gSchedRingIdx;
    if ((D_802B9C00[idx] < 30) && (D_802B9C18[idx] < 30)) {
        if (arg0 == FALSE) {
            var_v0 = gSchedCpuEvents[idx];
            var_v0 = &var_v0[D_802B9C00[idx]++];
        } else {
            var_v0 = gSchedRspEvents[idx];
            var_v0 = &var_v0[D_802B9C18[idx]++];
        }
        var_v0->timestamp = uvClkGetSec(UV_CLKID_SCHED);
        var_v0->eventID = eventID;
        var_v0->unkC = (gSchedRspStatus << 0x18) | (gSchedRdpStatus << 0x10) | (D_802B9C6B << 1) | D_802B9C6C;
    }
}
