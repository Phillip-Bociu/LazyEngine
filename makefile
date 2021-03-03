bin/lazy: bin-int/test.o bin-int/Renderer.o bin-int/App.o bin-int/Window.o 
	clang++  -lvulkan -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi   -o bin/lazy  bin-int/test.o bin-int/Renderer.o bin-int/App.o bin-int/Window.o 
bin-int/test.o: ./src/test.cpp
	clang++ -std=c++17 -D DEBUG -c -I./  ./src/test.cpp -o bin-int/test.o
bin-int/Renderer.o: ./src/Renderer.cpp
	clang++ -std=c++17 -D DEBUG -c -I./  ./src/Renderer.cpp -o bin-int/Renderer.o
bin-int/App.o: ./src/App.cpp
	clang++ -std=c++17 -D DEBUG -c -I./  ./src/App.cpp -o bin-int/App.o
bin-int/Window.o: ./src/Window.cpp
	clang++ -std=c++17 -D DEBUG -c -I./  ./src/Window.cpp -o bin-int/Window.o
clean:
	rm ./bin/lazy ./bin-int/*.o