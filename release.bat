@ECHO OFF
SetLocal EnableDelayedExpansion

SET assembly=lzy
SET compilerFlags=-shared -Ofast -std=c99 -fdeclspec -fvisibility=hidden -DLEXPORT -msse -mfma -D_CRT_SECURE_NO_WARNINGS

SET includeFlags=-Iheaders/ -I%VULKAN_SDK%/Include -I./
SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib

SET compilerFlags2= -std=c99 -Ofast -fdeclspec -fvisibility=hidden -msse -mfma -D_CRT_SECURE_NO_WARNINGS


REM echo clang++ deps/LzyDeps.cpp %includeFlags% -g -std=c++17 -D_CRT_SECURE_NO_WARNINGS -c -o bin/cppdeps.o
REM clang++ deps/LzyDeps.cpp %includeFlags% -g -std=c++17 -D_CRT_SECURE_NO_WARNINGS -c -o bin/cppdeps.o

echo clang bin/cppdeps.o src/LzyEngine_win32.c %compilerFlags% -o bin/%assembly%.dll %includeFlags% %linkerFlags%

clang bin/cppdeps.o src/LzyEngine_win32.c %compilerFlags% -o bin/%assembly%.dll %includeFlags% %linkerFlags%

echo Building Test 
echo clang test/test.c %compilerFlags2% -o bin/test.exe %includeFlags% %linkerFlags% -Lbin -l%assembly%
clang test/test.c %compilerFlags2% -o bin/test.exe %includeFlags% %linkerFlags% -Lbin -l%assembly%

pause
exit