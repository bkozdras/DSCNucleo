#ifndef _SEGMENTS_MANAGER_H_

#define _SEGMENTS_MANAGER_H_

#include "Defines/CommonDefines.h"
#include "SharedDefines/SSegmentData.h"
#include "System/ThreadMacros.h"
#include "stdbool.h"

THREAD_PROTOTYPE(StaticSegmentProgramExecutor)

void SegmentsManager_setup(void);
void SegmentsManager_initialize(void);

bool SegmentsManager_modifyRegisteredSegment(SSegmentData* data);
bool SegmentsManager_registerNewSegment(SSegmentData* data);
bool SegmentsManager_deregisterSegment(u16 number);
u16 SegmentsManager_getNumberOfRegisteredSegments(void);

void SegmentsManager_registerSegmentStartedIndCallback(void (*callback)(u8, u8));
void SegmentsManager_deregisterSegmentStartedIndCallback(void);
void SegmentsManager_registerSegmentsProgramDoneIndCallback(void (*callback)(u8, u8));
void SegmentsManager_deregisterSegmentsProgramDoneIndCallback(void);

bool SegmentsManager_startProgram(void);
bool SegmentsManager_stopProgram(void);
void SegmentsManager_clearProgram(void);

#endif
