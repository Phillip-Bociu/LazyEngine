#include "Lzy.h"
#ifdef __linux__
#include<GLFW/glfw3.h>

typedef struct LzyWindow_impl
{
	GLFWwindow* pGLwindow;
}LzyWindow_impl;


b8 lzy_create_window(LzyWindow* pWindow, u32 uResX, u32 uResY)
{
	*pWindow = malloc(sizeof(LzyWindow_impl));	



	return true;
}

#endif
