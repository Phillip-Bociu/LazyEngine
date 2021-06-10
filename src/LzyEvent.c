#include "LzyEvent.h"
#include "LzyMemory.h"

typedef struct LzyEventListener
{
	LzyfpOnEvent fpCallback;
	void* pListener;
}LzyEventListener;

typedef struct LzyEventSystem
{
	u64 uCoreListenerCount;
	LzyEventListener pCoreListeners[LZY_MAX_CORE_LISTENERS];
	u8 vUserListeners[];
}LzyEventSystem;

global b8 bIsInitialized = false;
global LzyEventSystem* pEventSystem;

b8 lzy_event_init()
{
}
void lzy_event_shutdown()
{

}

b8 lzy_event_core_register(LzyfpOnEvent fpOnEvent)
{

}
b8 lzy_event_core_deregister(LzyfpOnEvent fpOnEvent);

b8 lzy_event_reserve_slots(u64 uSlots);
b8 lzy_event_register(LzyfpOnEvent fpOnEvent);
b8 lzy_event_deregister(LzyfpOnEvent fpOnEvent);

b8 lzy_event_emit(u16 uCode, void *pSender, LzyEventData eData);