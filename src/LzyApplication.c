#include "Lzy.h"


global b8 bIsInitialized = false;
global LzyApplication lzyApp;


b8 lzy_application_create(LzyApplicationConfig* pAppConfig)
{
    if(bIsInitialized)
    {
        LERROR("Application was already created");
        return false;
    }


	LTRACE("lol %f", 3.14f);
	LINFO("lol %f", 3.14f);
	LWARN("lol %f", 3.14f);
	LERROR("lol %f", 3.14f);
	LFATAL("lol %f", 3.14f);
	LASSERT(false, "Assertion Test");
	if(!lzy_window_create(&lzyApp.pWindow, pAppConfig->pApplicationName, pAppConfig->uResX ,pAppConfig->uResY))
    {
        return false;
    }
    lzyApp.fLastTime = lzy_get_time();
    lzyApp.uResX = pAppConfig->uResX;
    lzyApp.uResY = pAppConfig->uResY;
    lzyApp.bIsRunning = true;
    lzyApp.bIsSuspended = false;
    
    bIsInitialized = true;
    return true;
}


b8 lzy_application_run()
{
	while(lzyApp.bIsRunning)
	{
		lzyApp.bIsRunning = !lzy_window_poll_events(lzyApp.pWindow);
	}

}