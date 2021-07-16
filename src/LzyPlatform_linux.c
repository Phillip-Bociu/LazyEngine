#ifdef __linux__
#include "LzyPlatform.h"
#include "LzyLog.h"
#include "LzyInput.h"

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
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

b8 lzy_platform_change_title(LzyPlatform platform, const char* pWindowTitle)
{
    LzyWindow_impl* pState = platform;
    xcb_change_property(pState->pConnection,
						XCB_PROP_MODE_REPLACE,
						pState->window,
						XCB_ATOM_WM_NAME,
						XCB_ATOM_STRING,
						8,
						strlen(pWindowTitle),
						pWindowTitle);
    return true;
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

internal_func LzyKeyState keysym_to_lzy_keycode(u32 keyCode)
{
    switch(keyCode)
    {
        case XK_a:
        case XK_A:
        return LZY_KEY_CODE_A;
        case XK_b:
        case XK_B:
        return LZY_KEY_CODE_B;
        case XK_c:
        case XK_C:
        return LZY_KEY_CODE_C;
        case XK_d:
        case XK_D:
        return LZY_KEY_CODE_D;
        case XK_e:
        case XK_E:
        return LZY_KEY_CODE_E;
        case XK_f:
        case XK_F:
        return LZY_KEY_CODE_F;
        case XK_g:
        case XK_G:
        return LZY_KEY_CODE_G;
        case XK_h:
        case XK_H:
        return LZY_KEY_CODE_H;
        case XK_i:
        case XK_I:
        return LZY_KEY_CODE_I;
        case XK_j:
        case XK_J:
        return LZY_KEY_CODE_J;
        case XK_k:
        case XK_K:
        return LZY_KEY_CODE_K;
        case XK_l:
        case XK_L:
        return LZY_KEY_CODE_L;
        case XK_m:
        case XK_M:
        return LZY_KEY_CODE_M;
        case XK_n:
        case XK_N:
        return LZY_KEY_CODE_N;
        case XK_o:
        case XK_O:
        return LZY_KEY_CODE_O;
        case XK_p:
        case XK_P:
        return LZY_KEY_CODE_P;
        case XK_q:
        case XK_Q:
        return LZY_KEY_CODE_Q;
        case XK_r:
        case XK_R:
        return LZY_KEY_CODE_R;
        case XK_s:
        case XK_S:
        return LZY_KEY_CODE_S;
        case XK_t:
        case XK_T:
        return LZY_KEY_CODE_T;
        case XK_u:
        case XK_U;
        return LZY_KEY_CODE_U;
        case XK_v:
        case XK_V:
        return LZY_KEY_CODE_V;
        case XK_w:
        case XK_W:
        return LZY_KEY_CODE_W;
        case XK_x:
        case XK_X:
        return LZY_KEY_CODE_X;
        case XK_y:
        case XK_Y:
        return LZY_KEY_CODE_Y;
        case XK_z:
        case XK_Z:
        return LZY_KEY_CODE_Z;
        case XK_space:
        return LZY_KEY_CODE_SPACE;
        case XK_Return:
        return LZY_KEY_CODE_RETURN;
        case XK__Escape:
        return LZY_KEY_CODE_ESCAPE;
        case XK_Shift_L:
        return LZY_KEY_CODE_LSHIFT;
        case XK_Shift_R
        return LZY_KEY_CODE_RSHIFT;
        case XK_Control_L:
        return LZY_KEY_CODE_LCTRL;
        case XK_Control_R:
        return LZY_KEY_CODE_RCTRL;
        case XK_Alt_L:
        return LZY_KEY_CODE_LALT;
        case XK_Alt_R:
        return LZY_KEY_CODE_RALT;
        case XK_Tab:
        return LZY_KEY_CODE_TAB;
        case XK_1:
        return LZY_KEY_CODE_1;
        case XK_2:
        return LZY_KEY_CODE_2;
        case XK_3:
        return LZY_KEY_CODE_3;
        case XK_4:
        return LZY_KEY_CODE_4;
        case XK_5:
        return LZY_KEY_CODE_5;
        case XK_6:
        return LZY_KEY_CODE_6;
        case XK_7:
        return LZY_KEY_CODE_7;
        case XK_8:
        return LZY_KEY_CODE_8;
        case XK_9:
        return LZY_KEY_CODE_9;
        case XK_0:
        return LZY_KEY_CODE_0;
        case XK_F1:
        return LZY_KEY_CODE_F1;
        case XK_F2:
        return LZY_KEY_CODE_F2;
        case XK_F3:
        return LZY_KEY_CODE_F3;
        case XK_F4:
        return LZY_KEY_CODE_F4;
        case XK_F5:
        return LZY_KEY_CODE_F5;
        case XK_F6:
        return LZY_KEY_CODE_F6;
        case XK_F7:
        return LZY_KEY_CODE_F7;
        case XK_F8:
        return LZY_KEY_CODE_F8;
        case XK_F9:
        return LZY_KEY_CODE_F9;
        case XK_F10:
        return LZY_KEY_CODE_F10;
        case XK_F11
        return LZY_KEY_CODE_F11;
        case XK_F12
        return LZY_KEY_CODE_F12;
        case XK_period:
        return LZY_KEY_CODE_PERIOD;
        case XK_comma:
        return LZY_KEY_CODE_COMMA;
        case XK_slash:
        return LZY_KEY_CODE_SLASH;
        case XK_backslash:
        return LZY_KEY_CODE_BACKSLASH;
        case XK_Up:
        return LZY_KEY_CODE_UP;
        case XK_Down:
        return LZY_KEY_CODE_DOWN;
        case XK_Left
        return LZY_KEY_CODE_LEFT;
        case XK_Right
        return LZY_KEY_CODE_RIGHT;
    }
    return LZY_KEY_CODE_RANGE;
}

internal_func LzyMouseButtonCode button_index_to_lzy_keycode(u32 uButtonIndex)
{
    switch(uButtonIndex)
    {
        case XCB_BUTTON_INDEX_1:
        return LZY_MOUSE_BUTTON_CODE_LEFT;
        case XCB_BUTTON_INDEX_2:
        return LZY_MOUSE_BUTTON_CODE_MIDDLE;
        case XCB_BUTTON_INDEX_3:
        return LZY_MOUSE_BUTTON_CODE_RIGHT;
    }
    return LZY_MOUSE_BUTTON_CODE_RANGE;
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
                xcb_key_press_event_t* pKeyEvent = pEvent;
                KeySym keySym = XkbKeycodeToKeysym(pState->pDisplay,
                                                   pKeyEvent->detail,
                                                   0,
                                                   pKeyEvent->detail & ShiftMask ? 1 : 0);
                
                LzyKeyCode keyCode = keysym_to_lzy_keycode(keySym);
                if(lzy_input_get_key_state(keyCode) != LZY_KEY_STATE_HOLD)
                    lzy_input_set_key_state(keyCode, LZY_KEY_STATE_PRESS);
            }
            break;
            case XCB_KEY_RELEASE:
            {
                xcb_key_press_event_t* pKeyEvent = pEvent;
                KeySym keySym = XkbKeycodeToKeysym(pState->pDisplay,
                                                   pKeyEvent->detail,
                                                   0,
                                                   pKeyEvent->detail & ShiftMask ? 1 : 0);
                
                LzyKeyCode keyCode = keysym_to_lzy_keycode(keySym);
                lzy_input_set_key_state(keyCode, LZY_KEY_STATE_RELEASE);
            }
            break;
            case XCB_BUTTON_PRESS:
            {
                xcb_button_press_event_t* pButtonEvent = pEvent;
                LzyMouseButtonCode buttonCode = button_index_to_lzy_keycode(pButtonEvent->details);
                if(lzy_get_mouse_button_state(buttonCode) != LZY_KEY_STATE_HOLD)
                {
                    lzy_set_mouse_button_state(buttonCode, LZY_KEY_STATE_PRESS);
                }
            }
            break;
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_press_event_t* pButtonEvent = pEvent;
                LzyMouseButtonCode buttonCode = button_index_to_lzy_keycode(pButtonEvent->details);
                lzy_set_mouse_button_state(buttonCode, LZY_KEY_STATE_RELEASE);
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
	return malloc(uSize);
}

u16 lzy_platform_get_number_of_threads()
{
	return get_nprocs(); 
}

void lzy_platform_free(void* ptr, u64 uSize, u8 uAlignment)
{
	free(ptr);
}

void* lzy_platform_realloc(void* ptr, u64 uSize)
{
	return realloc(ptr, uSize);
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
