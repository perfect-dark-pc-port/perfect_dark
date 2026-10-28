#ifndef _IN_BOOT_SCHED_H
#define _IN_BOOT_SCHED_H
#include <ultra64.h>
#include <PR/ultrasched.h>
#include "types.h"

void schedSetCrashEnable2(s32 enable);
void schedSubmitTask(OSSched *sc, OSScTask *t);
void __scHandleRetrace(OSSched *sc);
void __scHandleRSP(OSSched *sc);
void __scHandleRDP(OSSched *sc);
void __scHandleTasks(OSSched *sc);
void __scAppendList(OSSched *sc, OSScTask *t);
void __scExec(OSSched *sc, OSScTask *sp, OSScTask *dp);
void __scMain(void *arg);
void __scYield(OSSched *sc);
void schedInitArtifacts(void);
void schedResetArtifacts(void);
struct artifact *schedGetWriteArtifacts(void);
struct artifact *schedGetFrontArtifacts(void);
void schedIncrementWriteArtifacts(void);
void schedIncrementFrontArtifacts(void);
void schedUpdatePendingArtifacts(void);
void schedConsiderScreenshot(void);
void schedSetCrashedUnexpectedly(bool enable);
void schedStartFrame(OSSched *sc);
void schedEndFrame(OSSched *sc);
#ifndef PLATFORM_N64
u8 *schedGetPlayerLightTable(void);
s32 *schedGetPlayerSunTimers(void);
#endif

#endif
