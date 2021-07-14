@ECHO OFF
SetLocal EnableDelayedExpansion

SET assembly=lzyd
SET compilerFlags=-g -shared  -std=c99 -fdeclspec -fvisibility=hidden -DLEXPORT -D_DEBUG -msse -mfma -D_CRT_SECURE_NO_WARNINGS

SET includeFlags=-Iheaders/ -I%VULKAN_SDK%/Include -I./
SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib

SET compilerFlags2= -g -std=c99 -fdeclspec -fvisibility=hidden -D_DEBUG -msse -mfma -D_CRT_SECURE_NO_WARNINGS
echo Building Engine

REM echo clang++ deps/LzyDeps.cpp %includeFlags% -g -std=c++17 -D_CRT_SECURE_NO_WARNINGS -c -o bin/cppdepsd.o
REM clang++ deps/LzyDeps.cpp %includeFlags% -g -std=c++17 -D_CRT_SECURE_NO_WARNINGS -c -o bin/cppdepsd.o

echo clang bin/cppdepsd.o src/LzyEngine_win32.c %compilerFlags% -o bin/%assembly%.dll %includeFlags% %linkerFlags%
clang bin/cppdepsd.o src/LzyEngine_win32.c %compilerFlags% -o bin/%assembly%.dll %includeFlags% %linkerFlags%

echo Building Test 
echo clang test/test.c %compilerFlags2% -o bin/testd.exe %includeFlags% %linkerFlags% -Lbin -l%assembly%
clang test/test.c %compilerFlags2% -o bin/testd.exe %includeFlags% %linkerFlags% -Lbin -l%assembly%

pause
exit