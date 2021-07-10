@ECHO OFF
SetLocal EnableDelayedExpansion
REM Get a list of all the .c files.
set cFilenames=
echo C Sources:

FOR /R "src/" %%g in (*.c) do (
     echo %%g
     set cFilenames=!cFilenames! %%g
)

echo C Deps:
FOR /R "deps/" %%g in (*.c) do (
     echo %%g
     set cFilenames=!cFilenames! %%g
)

SET assembly=lzy
SET compilerFlags=-shared -Ofast -std=c99 -fdeclspec -fvisibility=hidden -DLEXPORT -msse -mfma -D_CRT_SECURE_NO_WARNINGS

SET includeFlags=-Iheaders/ -I%VULKAN_SDK%/Include -I./
SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib

SET compilerFlags2= -std=c99 -Ofast -fdeclspec -fvisibility=hidden -msse -mfma -D_CRT_SECURE_NO_WARNINGS

echo Building Engine
clang %cFilenames% %compilerFlags% -o bin/%assembly%.dll %includeFlags% %linkerFlags%

echo Building Test 
clang test/test.c %compilerFlags2% -o bin/test.exe %includeFlags% %linkerFlags% -Lbin -l%assembly%
pause
exit