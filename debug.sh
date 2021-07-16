set echo on
echo Building test

export LD_LIBRARY_PATH=.

assembly="lzyd"
compilerFlags="-g -shared -fPIC -fdeclspec  -fvisibility=hidden -msse -mfma -D_DEBUG -DLEXPORT -std=gnu99"
compilerFlags2="-g  -msse -mfma -D_DEBUG  -std=gnu99"
includeFlags="-Iheaders/ -I$VULKAN_SDK/Include -I./"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -lpthread -lstdc++"


echo Building Dependencies...
echo clang++ deps/LzyDeps.cpp $includeFlags -phtread  -std=c++17 -g -fPIC -c -o bin/cppdepsd.o
clang++ deps/LzyDeps.cpp $includeFlags -g -pthread  -std=c++17 -c -fPIC -o bin/cppdepsd.o

echo Building Engine...
echo clang bin/cppdepsd.o src/LzyEngine_linux.c $compilerFlags $includeFlags $linkerFlags -o bin/$assembly.so
clang bin/cppdepsd.o src/LzyEngine_linux.c $compilerFlags $includeFlags $linkerFlags -o bin/$assembly.so

echo Bulding Test...
echo clang test/test.c $compilerFlags2 -o bin/testd $includeFlags $linkerFlags -Lbin/ bin/$assembly.so 
clang test/test.c $compilerFlags2 -o bin/testd $includeFlags $linkerFlags -Lbin/ bin/$assembly.so

echo "Press any key to continue"
while [ true ] ; do
read -t 3 -n 1
if [ $? = 0 ] ; then
exit ;
fi
done