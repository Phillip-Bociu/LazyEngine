#include "LzyEvent.h"
#include "LzyMemory.h"

typedef struct LzyEventListener
{
	LzyfpOnEvent fpCallback;
	void *pListener;
} LzyEventListener;

typedef struct LzyEventSystem
{
	u64 uCoreListenerCount;
	u64 uUserListenerCount;
	LzyEventListener pCoreListeners[LZY_MAX_CORE_LISTENERS];
	LzyEventListener pUserListeners[LZY_MAX_USER_LISTENERS];
} LzyEventSystem;

global b8 bIsInitialized = false;
global LzyEventSystem *pEventSystem;

b8 lzy_event_init()
{
	pEventSystem = lzy_alloc(sizeof(LzyEventSystem), 8, LZY_MEMORY_TAG_EVENT_SYSTEM);
	bIsInitialized = true;
	LINFO("Event Subsystem Initialized");
	return true;
}

void lzy_event_shutdown()
{
}

b8 lzy_event_core_register(LzyfpOnEvent fpOnEvent, void *pListener)
{
	LCOREASSERT(pEventSystem->uCoreListenerCount < LZY_MAX_CORE_LISTENERS, "Surpassed core event listener capacity!");
	pEventSystem->pCoreListeners[pEventSystem->uCoreListenerCount].fpCallback = fpOnEvent;
	pEventSystem->pCoreListeners[pEventSystem->uCoreListenerCount].pListener = pListener;
	pEventSystem->uCoreListenerCount++;

	return true;
}
b8 lzy_event_core_deregister(LzyfpOnEvent fpOnEvent, void *pListener)
{
	LCOREASSERT(pEventSystem->uCoreListenerCount > 0, "No more core listeners to deregister!");

	const LzyEventListener *const pBegin = pEventSystem->pCoreListeners;
	const LzyEventListener *const pEnd = pEventSystem->pCoreListeners + pEventSystem->uCoreListenerCount;

	LzyEventListener *pToRemove = pBegin;

	for (; pToRemove != pEnd; pToRemove++)
	{
		if (pToRemove->fpCallback == fpOnEvent &&
			pToRemove->pListener == pListener)
		{
			break;
		}
	}

	if (pToRemove == pEnd)
	{
		LWARN("Trying to remove core listener that does not exist, (func:%p, listener:%p)", fpOnEvent, pListener);
		return false;
	}

	for (; pToRemove != pEnd - 1; pToRemove++)
	{
		*pToRemove = *(pToRemove + 1);
	}
	pEventSystem->uCoreListenerCount--;
	return true;
}

b8 lzy_event_register(LzyfpOnEvent fpOnEvent, void *pListener)
{
	LASSERT(pEventSystem->uUserListenerCount < LZY_MAX_USER_LISTENERS, "Surpassed core event listener capacity!");
	pEventSystem->pUserListeners[pEventSystem->uUserListenerCount].fpCallback = fpOnEvent;
	pEventSystem->pUserListeners[pEventSystem->uUserListenerCount].pListener = pListener;
	pEventSystem->uUserListenerCount++;

	return true;
}
b8 lzy_event_deregister(LzyfpOnEvent fpOnEvent, void *pListener)
{
	LCOREASSERT(pEventSystem->uUserListenerCount > 0, "No more user listeners to deregister!");

	LzyEventListener *pToRemove = pEventSystem->pUserListeners;
	const LzyEventListener *pEnd = pEventSystem->pUserListeners + pEventSystem->uUserListenerCount;

	for (; pToRemove != pEnd; pToRemove++)
	{
		if (pToRemove->fpCallback == fpOnEvent &&
			pToRemove->pListener == pListener)
		{
			break;
		}
	}

	if (pToRemove == pEnd)
	{
		LCOREWARN("Trying to remove user listener that does not exist, (func:%p, listener:%p)", fpOnEvent, pListener);
		return false;
	}

	for (; pToRemove != pEnd - 1; pToRemove++)
	{
		*pToRemove = *(pToRemove + 1);
	}
	pEventSystem->uUserListenerCount--;

	return true;
}

b8 lzy_event_emit(u16 uCode, void *pSender, LzyEventData eData)
{
	const LzyEventListener *pBegin = pEventSystem->pCoreListeners;
	const LzyEventListener *pEnd   = pEventSystem->pCoreListeners + pEventSystem->uCoreListenerCount;

	for (; pBegin != pEnd; pBegin++)
	{
		if (pBegin->fpCallback(uCode, pSender, pBegin->pListener, eData))
		{
			return true;
		}
	}

	pBegin = pEventSystem->pUserListeners;
	pEnd   = pEventSystem->pUserListeners + pEventSystem->uUserListenerCount;

	for (; pBegin != pEnd; pBegin++)
	{
		if (pBegin->fpCallback(uCode, pSender, pBegin->pListener, eData))
		{
			break;
		}
	}

	return true;
}