#ifdef __linux__
#include "LzyPlatform.h"
#include "LzyLog.h"

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include<sys/sysinfo.h>

#include <string.h>
#include <stdlib.h>

typedef struct LzyWindow_impl
{
	Display *pDisplay;
	xcb_connection_t *pConnection;
	xcb_window_t window;
	xcb_screen_t *pScreen;
	xcb_atom_t wmProtocols;
	xcb_atom_t wmDeleteWin;
} LzyWindow_impl;

void lzy_platform_shutdown(LzyPlatform platform)
{
	LzyWindow_impl *pState = platform;
	XAutoRepeatOn(pState->pDisplay);
}


void lzy_platform_get_framebuffer_size(LzyPlatform platform, u16* pX, u16* pY)
{

	LzyWindow_impl *pState = platform;
	
	xcb_get_geometry_cookie_t cookie =  xcb_get_geometry(pState->pConnection, pState->window);

	xcb_get_geometry_reply_t* reply = xcb_get_geometry_reply(pState->pConnection, cookie, NULL);

	if(pX)
		*pX = reply->width;
	if(pY)
		*pY = reply->height;

	free(reply);
}

void lzy_platform_get_surface_create_info(LzyPlatform platform, LzyWindowSurfaceCreateInfo* pSurface)
{
	LzyWindow_impl* pState = platform;
	pSurface->sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	pSurface->flags = 0;
	pSurface->connection = pState->pConnection;
	pSurface->window = pState->window;
}

VkResult lzy_platform_create_surface(VkInstance instance, const LzyWindowSurfaceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocs, VkSurfaceKHR* pSurface)
{
	return vkCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocs, pSurface);
}


b8 lzy_platform_create(LzyPlatform *pPlatform, const char *pWindowTitle, u16 uResX, u16 uResY)
{
	*pPlatform = malloc(sizeof(LzyWindow_impl));
	LzyWindow_impl *pState = *pPlatform;

	pState->pDisplay = XOpenDisplay(NULL);
	//XAutoRepeatOff(pState->pDisplay);

	pState->pConnection = XGetXCBConnection(pState->pDisplay);

	if (xcb_connection_has_error(pState->pConnection))
	{
		LCOREFATAL("%s", "Failed to connect to X server, cannot create window");
		return false;
	}

	const xcb_setup_t *pSetup = xcb_get_setup(pState->pConnection);

	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(pSetup);
	pState->pScreen = iter.data;

	pState->window = xcb_generate_id(pState->pConnection);

	u32 uEventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
			   XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
			   XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
			   XCB_EVENT_MASK_STRUCTURE_NOTIFY;

	u32 uEventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

	u32 uValueList[] = {pState->pScreen->black_pixel, uEventValues};

	xcb_void_cookie_t cookie = xcb_create_window(
		pState->pConnection,
		XCB_COPY_FROM_PARENT,
		pState->window,
		pState->pScreen->root,
		500, 500,
		uResX, uResY,
		200,
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		pState->pScreen->root_visual,
		uEventMask,
		uValueList);

	xcb_change_property(pState->pConnection,
						XCB_PROP_MODE_REPLACE,
						pState->window,
						XCB_ATOM_WM_NAME,
						XCB_ATOM_STRING,
						8,
						strlen(pWindowTitle),
						pWindowTitle);

	xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
		pState->pConnection,
		0,
		strlen("WM_DELETE_WINDOW"),
		"WM_DELETE_WINDOW");

	xcb_intern_atom_cookie_t wmProtocolCookie = xcb_intern_atom(
		pState->pConnection,
		0,
		strlen("WM_PROTOCOLS"),
		"WM_PROTOCOLS");

	xcb_intern_atom_reply_t *pWmDeleteReply = xcb_intern_atom_reply(
		pState->pConnection,
		wmDeleteCookie,
		NULL);
	xcb_intern_atom_reply_t *pWmProtocolReply = xcb_intern_atom_reply(
		pState->pConnection,
		wmProtocolCookie,
		NULL);

	pState->wmDeleteWin = pWmDeleteReply->atom;
	pState->wmProtocols = pWmProtocolReply->atom;

	xcb_change_property(
		pState->pConnection,
		XCB_PROP_MODE_REPLACE,
		pState->window,
		pWmProtocolReply->atom,
		4,
		32,
		1,
		&pWmDeleteReply->atom);

	xcb_map_window(pState->pConnection, pState->window);

	i32 iStreamResult = xcb_flush(pState->pConnection);
	if (iStreamResult <= 0)
	{
		LCOREFATAL("Could not flush XCB stream, error %d", iStreamResult);
		return false;
	}

	return true;
}

b8 lzy_platform_poll_events(LzyPlatform platform)
{

	LzyWindow_impl *pState = (LzyWindow_impl *)platform;

	xcb_generic_event_t *pEvent;
	xcb_client_message_event_t *pClientMessage;

	b8 bShouldClose = false;

	while (true)
	{
		pEvent = xcb_poll_for_event(pState->pConnection);
		if (!pEvent)
			break;

		switch (pEvent->response_type & ~0x80)
		{
		case XCB_KEY_PRESS:
		{
			LINFO("%s", "Key Pressed");
		}
		break;
		case XCB_KEY_RELEASE:
		{
			LINFO("%s", "Key Released");
		}
		break;
		case XCB_BUTTON_PRESS:
		{
			LINFO("%s", "Button Pressed");
		}
		break;
		case XCB_BUTTON_RELEASE:
		{
			LINFO("%s", "Button Released");
		}
		break;
		case XCB_MOTION_NOTIFY:
		{
		}
		break;
		case XCB_CONFIGURE_NOTIFY:
		{
		}
		break;
		case XCB_CLIENT_MESSAGE:
		{
			pClientMessage = (xcb_client_message_event_t *)pEvent;
			if (pClientMessage->data.data32[0] == pState->wmDeleteWin)
			{
				bShouldClose = true;
			}
		}
		break;
		}
		free(pEvent);
	}

	return bShouldClose;
}

f64 lzy_platform_get_time()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + t.tv_usec * 1e-6;
}

void lzy_platform_sleep(u64 uMs)
{
#if _POSIX_C_SOURCE >= 199309L
	struct timespec ts;
	ts.tv_sec = uMs / 1000;
	ts.tv_nsec = (uMs % 1000) * 1000 * 1000;
	nanosleep(&ts, 0);
#else
	if (uMs >= 1000)
	{
		sleep(uMs / 1000);
	}
	usleep((uMs % 1000) * 1000);
#endif
}

void *lzy_platform_alloc(u64 uSize, u8 uAlignment)
{
	return mmap(NULL, uSize, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANON, -1, 0);
}

u16 lzy_platform_get_number_of_threads()
{
	return get_nprocs(); 
}

void lzy_platform_free(void* ptr, u64 uSize, u8 uAlignment)
{
	munmap(ptr, uSize);
}

void *lzy_platform_memcpy(void *pDst, void *pSrc, u64 uSize)
{
	return memcpy(pDst, pSrc, uSize);
}

void *lzy_platform_memset(void *pDst, u8 uVal, u64 uSize)
{
	return memset(pDst, uVal, uSize);
}

void *lzy_platform_memzero(void *pDst, u64 uSize)
{
	return memset(pDst, 0, uSize);
}
#endif
