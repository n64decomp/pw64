#ifndef PILOTWINGS64_UV_AUDIO_H
#define PILOTWINGS64_UV_AUDIO_H

#include <uv_common.h>
#include <uv_matrix.h>
#include <uv_sched.h>

#define AUDIO_BUF_SIZE 0x4B00
#define DMA_BUF_SIZE 0x800
#define NUM_DMA_BUFFERS 48
#define NUM_DMA_MESSAGES 48
#define AUDIO_QUIT_MSG 10
#define EXTRA_SAMPLES 100
#define OUTPUT_RATE 22050
#define FRAME_LAG 2
#define MAX_MESGS 8

// -----------------------------------------------------------------------------
// Audio Manager API
// -----------------------------------------------------------------------------

typedef struct UnkStruct_80200144 {
    Mtx4F mat;
    f32 unk40;
    f32 unk44;
    f32 unk48;
    s32 unk4C;
} UnkStruct_80200144_t;

extern UnkStruct_80200144_t D_80250E80[];

typedef struct {
    ALLink node;
    u32 startAddr;
    u32 lastFrame;
    char *ptr;
} AMDMABuffer;

typedef struct {
    u8 initialized;
    AMDMABuffer *firstUsed;
    AMDMABuffer *firstFree;
} AMDMAState;

typedef union {    
    struct {
        short     type;
    } gen;
    struct {
        short     type;
        struct    AudioInfo *info;
    } done;
    OSScMsg       app;
} AudioMsg;

typedef struct AudioInfo {
    s16* data;
    s16 frameSamples;
    OSScTask task;
    AudioMsg msg;
} AudioInfo; // size 0x80

typedef struct AMAudioMgr {
    Acmd* ACMDList[2];
    AudioInfo* audioInfo[3];
    OSThread thread;
    OSMesgQueue audioFrameMsgQ;
    OSMesg audioFrameMsgBuf[MAX_MESGS];
    OSMesgQueue audioReplyMsgQ;
    OSMesg audioReplyMsgBuf[MAX_MESGS];
    ALGlobals g;
} AMAudioMgr;

extern ALSndId *gSndVoiceTable;
extern ALBank* gAudioSXBank;
extern ALSndPlayer* gSndPlayer;

void uvaManagerInit(void);
void uvaManager_80204518(s32 arg0);
void amCreateAudioMgr(ALSynConfig* c, OSPri priority);

// -----------------------------------------------------------------------------
// Audio Emitter API
// -----------------------------------------------------------------------------

typedef struct {
/* 00 */ Mtx4F m1;
/* 40 */ Vec3F unk40;
/* 4C */ Vec3F unk4C;
/* 58 */ Vec3F unk58;
/* 64 */ Vec3F unk64;
/* 70 */ f32 vol;
/* 74 */ f32 pitch;
/* 78 */ f32 pan;
/* 7C */ f32 mix;
/* 80 */ f32 unk80;
/* 84 */ f32 unk84;
/* 88 */ s32 priority;
/* 8C */ u8 sound;
/* 8D */ u8 unk8D;
/* 8E */ u8 unk8E;
/* 8F */ u8 unk8F;
/* 90 */ f32 near;
/* 94 */ f32 far;
/* 98 */ s32 attr; // EMITTER_ATTR_*
/* 9C */ f32 playPitch;
/* A0 */ s32 playState;
/* A4 */ s32 playVolume;
/* A8 */ s32 state; // EMITTER_STATE_*
/* AC */ u8 playVoice;
/* AD */ u8 playPan;
/* AE */ u8 playMix;
/* AF */ u8 playTimeout;
} uvaEmitter;

#define EMITTER_STATE_RELEASED 0
#define EMITTER_STATE_TRIGGERED 0x10

#define EMITTER_ATTR_1          0x01
#define EMITTER_ATTR_2          0x02
#define EMITTER_ATTR_4          0x04
#define EMITTER_ATTR_8          0x08
#define EMITTER_ATTR_ONESHOT    0x10
#define EMITTER_ATTR_INIT       0x20

#define EMITTER_PROPID_END      0
#define EMITTER_PROPID_NEAR     1 // type:f64
#define EMITTER_PROPID_FAR      2 // type:f64
#define EMITTER_PROPID_3        3 // type:f64
#define EMITTER_PROPID_4        4 // type:f64
#define EMITTER_PROPID_ATTR     5 // type:s32

#define EMITTER_PROP_END        EMITTER_PROPID_END
#define EMITTER_PROP_NEAR(x)    EMITTER_PROPID_NEAR, (x)
#define EMITTER_PROP_FAR(x)     EMITTER_PROPID_FAR, (x)
#define EMITTER_PROP_3(x)       EMITTER_PROPID_3, (x)
#define EMITTER_PROP_4(x)       EMITTER_PROPID_4, (x)
#define EMITTER_PROP_ATTR(x)    EMITTER_PROPID_ATTR, (x)

void uvEmitterPrintf(const char* fmt, ...);
void uvEmitterInitTable(void);
void uvEmitterInit(uvaEmitter* emitter);
u8   uvEmitterLookup(void);
void uvEmitterFromModel(u8 emitterId, u8 modelId);
void uvEmitterSetMatrix(u8 emitterId, Mtx4F* src);
void uvEmitterGetMatrix(u8 emitterId, Mtx4F* dst);
void uvEmitter_80201494(u8 emitterId, Vec3F arg1, Vec3F arg4);
void uvEmitterSetVol(u8 emitterId, f32 vol);
f32  uvEmitterGetVol(u8 emitterId);
void uvEmitterSetPan(u8 emitterId, f32 pan);
void uvEmitterSetPitch(u8 emitterId, f32 pitch);
void uvEmitterSetPri(u8 emitterId, s32 pri);
void uvEmitterProps(u8 emitterId, ...);
void uvEmitterTrigger(u8 emitterId);
void uvEmitterRelease(u8 emitterId);
void uvEmitterStatus(u8 arg0);
void uvSoundBegin(void);
void uvSoundEnd(void);
void uvEmitterFlush(u16 arg0);
void uvEmitter_80201D08(u8 emitterId, u16 arg1);
void _uvaUpdatePlayList(u8 emitterId);
void _uvaPlaylistRemove(u8 emitterId);
void _uvaPlay(void);
void _uvaStartVoice(u8 emitterId);
void _uvaUpdateVoice(u8 emitterId);
void _uvaStopVoice(u8 voiceId);
void _uvaStatus(u8 emitterId);

// -----------------------------------------------------------------------------
// Audio Event API
// -----------------------------------------------------------------------------

typedef void (*EventCallback_t)(s32, void*, s32);

typedef struct EventCallbackInfo {
    EventCallback_t cb;
    void* arg;
} EventCallbackInfo;

typedef struct UserEventCallbackInfo {
    EventCallbackInfo unk0[4];
    s32 count;
} UserEventCallbackInfo;

typedef struct SystemEventCallbackInfo {
    EventCallbackInfo unk0[10];
    s32 count;
} SystemEventCallbackInfo;

void uvEventInit(void);
void uvEventMaxCb(EventCallbackInfo arg0, ...);
void uvEventRemoveCb(EventCallbackInfo arg0, ...);
void uvEventPost(s32, s32);

// -----------------------------------------------------------------------------
// Audio Sequence API
// -----------------------------------------------------------------------------

void uvaSeqNew(s32);
void uvaSeqPlay(void);
void uvaSeqSetTempo(f32 tempo);
void uvaSeqSetVol(f32 vol);
void uvaSeqStop(void);

#endif // PILOTWINGS64_UV_AUDIO_H

