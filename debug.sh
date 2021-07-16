set echo on
echo Building test

export LD_LIBRARY_PATH=./bin/

assembly="lzyd"
compilerFlags="-g -shared -fPIC -fdeclspec  -msse -mfma -D_DEBUG -DLEXPORT -std=c99"
compilerFlags2="-g -fPIC -fdeclspec  -msse -mfma -D_DEBUG  -std=c99"
includeFlags="-Iheaders/ -I$VULKAN_SDK/Include -I./"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -lpthread"


echo Building Dependencies...
echo clang++ deps/LzyDeps.cpp $includeFlags -g -std=c++17 -fPIC -c -o bin/cppdepsd.o
clang++ deps/LzyDeps.cpp $includeFlags -g -std=c++17 -c -fPIC -o bin/cppdepsd.o

echo Building Engine...
echo clang bin/cppdepsd.o src/LzyEngine_linux.c $compilerFlags $includeFlags $linkerFlags -o bin/$assembly.so
clang bin/cppdepsd.o src/LzyEngine_linux.c $compilerFlags $includeFlags $linkerFlags -o bin/$assembly.so

echo Bulding Test...
echo clang test/test.c $compilerFlags2 -o bin/testd $includeFlags $linkerFlags -Lbin/  -Wl,-rpath, -l$assembly 
clang test/test.c $compilerFlags2 -o bin/testd $includeFlags $linkerFlags -Lbin/  -Wl,-rpath,   -l$assembly
