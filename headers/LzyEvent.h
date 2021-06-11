#pragma once
#include "LzyDefines.h"
#define LZY_MAX_CORE_LISTENERS 256
#define LZY_MAX_USER_LISTENERS 1024
typedef struct LzyEventData
{
	union
	{
		i64 i64[2];
		u64 u64[2];

		i32 i32[4];
		u32 u32[4];

		i16 i16[8];
		u16 u16[8];

		i8 i8[16];
		u8 u8[16];

		c8 c[16];
	};
} LzyEventData;

//TODO: define how each code modifies the eData parameter
typedef enum LzyEventCode
{
	LZY_EVENT_CODE_KEY_PRESS,
	LZY_EVENT_CODE_KEY_RELEASE,
	LZY_EVENT_CODE_BUTTON_PRESS,
	LZY_EVENT_CODE_BUTTON_RELEASE,
	LZY_EVENT_CODE_WINDOW_RESIZE,
	LZY_EVENT_CODE_WINDOW_CLOSE,
	LZY_EVENT_CODE_WINDOW_QUIT,
	LZY_EVENT_CODE_MOUSE_MOVE,
	LZY_EVENT_CODE_MOUSE_BUTTON_PRESS,
	LZY_EVENT_CODE_MOUSE_BUTTON_RELEASE,
	LZY_EVENT_CODE_CORE_MAX = 255,
	LZY_EVENT_CODE_MAX = 4096
} LzyEventCode;

typedef b8 (*LzyfpOnEvent)(u16 uCode, void *pSender, void* pListener, LzyEventData eData);

b8 lzy_event_init();
void lzy_event_shutdown();

b8 lzy_event_core_register(LzyfpOnEvent fpOnEvent, void* pListener);
b8 lzy_event_core_deregister(LzyfpOnEvent fpOnEvent, void* pListener);

LAPI b8 lzy_event_reserve_slots(u64 uSlots);
LAPI b8 lzy_event_register(LzyfpOnEvent fpOnEvent, void* pListener);
LAPI b8 lzy_event_deregister(LzyfpOnEvent fpOnEvent, void* pListener);

LAPI b8 lzy_event_emit(u16 uCode, void* pSender, LzyEventData eData);

