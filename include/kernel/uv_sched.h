#ifndef PILOTWINGS64_UV_SCHED_H
#define PILOTWINGS64_UV_SCHED_H

#include <uv_common.h>
#include <uv_matrix.h>
#include <PR/sched.h>

#define UV_MESG_DMA     0
#define UV_MESG_SCHED   2
#define UV_MESG_GFX     4

#define EVENT_ID_AUDIO_START 0x29
#define EVENT_ID_GFX_START 0x2A
#define EVENT_ID_GFX_DONE 0x2B
#define EVENT_ID_AUDIO_DONE 0x2C
#define EVENT_ID_GFX_YIELD 0x2D
#define EVENT_ID_GFX_SCHED 0x2E
#define EVENT_ID_AUDIO_SCHED 0x2F
#define EVENT_ID_GFX_RDP_DONE 0x30
#define EVENT_ID_YIELD_REQUEST 0x31
#define EVENT_ID_KERNEL_WAKEUP 0x32
#define EVENT_ID_USER1 0x33
#define EVENT_ID_USER2 0x34
#define EVENT_ID_USER3 0x35

typedef struct Unk8022B0A0 {
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
} Unk8022B0A0; // size: 0x10

typedef struct {
    f64 timestamp;
    s32 eventID;
    u32 unkC;
} ScheduleEventLog;

extern ScheduleEventLog gSchedCpuEvents[][30];
extern ScheduleEventLog gSchedRspEvents[][30];

extern OSSched gSchedInst;
extern s32 D_8024B260;

void _uvScDoneGfx(void);
void _uvScDoneAud(void);
void _uvScRunAud(void);
void _uvScRunGfx(void);
void _uvScDlistRecover(void);
void _uvScCreateScheduler(OSSched* sc, void* stack, s32 priority, u8 mode, u8 numFields);
void _uvScAddClient(OSSched *s, OSScClient *c, OSMesgQueue *msgQ);
OSMesgQueue* _uvScGetCmdQ(OSSched *s);
void _uvScMain(void* arg);
void _uvScHandleRetrace(void);
void _uvScHandleRSP(void);
void _uvScHandleRDP(void);
void _uvScHandleNMI(void);
void _uvScLogCpuEvent(s32 ring);
void _uvScLogIntoRing(void);
void uvSc_8022B0A0(Unk8022B0A0* arg0, Mtx4F* arg1);
void uvSc_8022BEB8(s32 arg0);
void uvSc_8022C34C(void);
void uvSc_8022C3C0(u8 arg0, s32 arg1);

#endif // PILOTWINGS64_UV_SCHED_H

