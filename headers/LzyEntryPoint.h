#pragma once

#include "LzyApplication.h"
#include "LzyLog.h"
#include "LzyGame.h"
#include "LzyVector.h"
#include <string.h>

extern b8 create_game(LzyGame *pGame);

int main()
{
    LzyGame game = {0};
    
    if (!create_game(&game))
    {
        LFATAL("%s","Could not create game!");
        return -1;
    }
    
    if (!game.fpOnResize ||
        !game.fpStart ||
        !game.fpUpdate ||
        !game.fpRender)
    {
        c8 buffer[1024] = "Some game function pointers not set:\n";
        if (!game.fpStart)
            strcat(buffer, "- fpStart\n");
        if (!game.fpUpdate)
            strcat(buffer, "- fpUpdate\n");
        if (!game.fpRender)
            strcat(buffer, "- fpRender\n");
        if (!game.fpOnResize)
            strcat(buffer, "- fpOnResize\n");
        LCOREFATAL("%s",buffer);
        return -2;
    }
    
    if(!lzy_application_create(&game))
    {
        LCOREFATAL("%s","Could not create application!");
        return 1;
    }
    
    if(!lzy_application_run())
    {
        LCOREFATAL("%s", "Application crashed!");
        return 2;
    }
    
    return 0;
}
