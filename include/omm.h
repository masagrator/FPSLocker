#pragma once

#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

Result ommInitialize(void);
void ommExit(void);
Service* ommGetServiceSession(void);
Result ommGetDefaultDisplayResolution(s32* width, s32* height);

#ifdef __cplusplus
} // extern "C"
#endif
