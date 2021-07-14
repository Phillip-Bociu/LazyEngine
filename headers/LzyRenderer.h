#pragma once
#include "LzyDefines.h"
#include "LzyPlatform.h"

b8 lzy_renderer_init();
void lzy_renderer_shutdown();
b8 lzy_renderer_loop(f64 fDeltaTime);
b8 lzy_renderer_recreate_swapchain();