#pragma once
#include "ipc.h"

Handle saltysd_orig;

Result SaltySD_Connect() {
    for (int i = 0; i < 200; i++) {
		if (!svcConnectToNamedPort(&saltysd_orig, "SaltySD"))
            return 0;
		svcSleepThread(1000*1000);
	}
    return 1;
}

Result SaltySD_Term()
{
	Result ret;
	IpcCommand c;

	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input
	{
		u64 magic;
		u64 cmd_id;
		u64 zero;
		u64 reserved[2];
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 0;
	raw->zero = 0;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) 
	{
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
		} *resp = (output*)r.Raw;

		ret = resp->result;
	}
	
	// Session terminated works too.
    svcCloseHandle(saltysd_orig);
	if (ret == 0xf601) return 0;

	return ret;
}

Result SaltySD_CheckIfSharedMemoryAvailable(ptrdiff_t *offset, u64 size)
{
	Result ret = 0;

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u64 size;
		u32 reserved[2];
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 6;
	raw->size = size;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 offset;
		} *resp = (output*)r.Raw;

		ret = resp->result;
		
		if (!ret)
		{
			*offset = resp->offset;
		}
	}
	
	return ret;
}

Result SaltySD_GetSharedMemoryHandle(Handle *retrieve)
{
	Result ret = 0;

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u32 reserved[4];
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 7;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 reserved[2];
		} *resp = (output*)r.Raw;

		ret = resp->result;
		
		if (!ret)
		{
			*retrieve = r.Handles[0];
		}
	}
	
	return ret;
}

Result SaltySD_SetDisplayRefreshRate(uint8_t refreshRate)
{
	Result ret = 0;

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u64 refreshRate;
		u64 reserved;
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 11;
	raw->refreshRate = refreshRate;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 reserved[2];
		} *resp = (output*)r.Raw;

		ret = resp->result;
	}
	
	return ret;
}

Result SaltySD_GetDisplayRefreshRate(uint8_t* refreshRate)
{
	Result ret = 0;

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u64 zero;
		u64 reserved;
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 10;
	raw->zero = 0;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 refreshRate;
			u64 reserved;
		} *resp = (output*)r.Raw;

		ret = resp->result;
		
		if (!ret)
		{
			*refreshRate = (uint8_t)(resp->refreshRate);
		}
	}
	
	return ret;
}

Result SaltySD_SetDisplaySync(bool isTrue)
{
	Result ret = 0;

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u64 value;
		u64 reserved;
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 12;
	raw->value = isTrue;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 reserved[2];
		} *resp = (output*)r.Raw;

		ret = resp->result;
	}
	
	return ret;
}

typedef bool DockedModeRefreshRateAllowed[5];

uint8_t DockedModeRefreshRateAllowedValues[] = { 40,
                                                 45,
                                                 50,
                                                 55,
                                                 60};

static_assert(sizeof(DockedModeRefreshRateAllowedValues) == sizeof(DockedModeRefreshRateAllowed));

Result SaltySD_SetAllowedDockedRefreshRates(DockedModeRefreshRateAllowed refreshRates)
{
	Result ret = 0;

	struct {
		unsigned int Hz_40: 1;
		unsigned int Hz_45: 1;
		unsigned int Hz_50: 1;
		unsigned int Hz_55: 1;
		unsigned int Hz_60: 1;
		unsigned int reserved: 27;
	} DockedRefreshRates;

	memset(&DockedRefreshRates, 0, sizeof(DockedRefreshRates));

	uint32_t value = 0;
	for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
		value |= ((uint32_t)refreshRates[i] << i);
	}
	memcpy(&DockedRefreshRates, &value, 4);

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u32 refreshRates;
		u32 reserved[3];
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 13;
	memcpy(&raw->refreshRates, &DockedRefreshRates, 4);

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 reserved[2];
		} *resp = (output*)r.Raw;

		ret = resp->result;
	}
	
	return ret;
}

Result SaltySD_SetDontForce60InDocked(bool isTrue)
{
	Result ret = 0;

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u64 value;
		u64 reserved;
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 14;
	raw->value = isTrue;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 reserved[2];
		} *resp = (output*)r.Raw;

		ret = resp->result;
	}
	
	return ret;
}

Result SaltySD_SetMatchLowestRR(bool isTrue)
{
	Result ret = 0;

	// Send a command
	IpcCommand c;
	ipcInitialize(&c);
	ipcSendPid(&c);

	struct input {
		u64 magic;
		u64 cmd_id;
		u64 value;
		u64 reserved;
	} *raw;

	raw = (input*)ipcPrepareHeader(&c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = 15;
	raw->value = isTrue;

	ret = ipcDispatch(saltysd_orig);

	if (R_SUCCEEDED(ret)) {
		IpcParsedCommand r;
		ipcParse(&r);

		struct output {
			u64 magic;
			u64 result;
			u64 reserved[2];
		} *resp = (output*)r.Raw;

		ret = resp->result;
	}
	
	return ret;
}