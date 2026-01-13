#define NX_SERVICE_ASSUME_NON_DOMAIN
#include <switch.h>
#include "service_guard.h"
#include "omm.h"

static Service g_ommSrv;

NX_GENERATE_SERVICE_GUARD(omm);

Result _ommInitialize(void) {
    return smGetService(&g_ommSrv, "omm");
}

void _ommCleanup(void) {
    serviceClose(&g_ommSrv);
}

Service* ommGetServiceSession(void) {
    return &g_ommSrv;
}

Result ommGetDefaultDisplayResolution(s32* width, s32* height) { //[3.0.0+]
    struct {
        s32 width;
        s32 height;
    } out;

    Result rc = serviceDispatchOut(&g_ommSrv, 11, out);

    if (R_SUCCEEDED(rc)) {
        *width = out.width;
        *height = out.height;
    }

    return rc;
}

Result ommGetOperationMode(AppletOperationMode* s) {

    u32 out;

    Result rc = serviceDispatchOut(&g_ommSrv, 0, out);

    if (R_SUCCEEDED(rc)) {
        *s = out;
    }

    return rc;
}