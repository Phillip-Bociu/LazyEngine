LIBS = -lglfw -lvulkan -lX11 -lpthread -lxcb -lX11-xcb -lxkbcommon
DCFLAGS = -std=gnu99 -D DEBUG -I./headers -c -O0 -Wall
DCFLAGS2 = -std=gnu99 -D DEBUG -I./headers -O0 -Wall -g

CFLAGS = -std=gnu99 -I./headers -c -O3 -Wall
CFLAGS2 = -std=gnu99 -I./headers -O3 -Wall

CC = gcc

RELEASE_DIR = bin-int/release
DEBUG_DIR = bin-int/debug

objs = $(RELEASE_DIR)/vector.o $(RELEASE_DIR)/utility.o $(RELEASE_DIR)/sparse_set.o $(RELEASE_DIR)/cvec.o $(RELEASE_DIR)/id_queue.o $(RELEASE_DIR)/entity_registry.o $(RELEASE_DIR)/allocation.o

#sources = src/vector.c

release: bin/liblzy.a test/test.c
	$(CC) test/test.c $(LIBS) -Lbin/ -llzy -o bin/test $(CFLAGS2)

debug: test/test.c
	$(CC) test/test.c -g $(LIBS) -o bin/testd $(DCFLAGS2)

bin/liblzy.a: $(objs)
	ar rcs $@ $^

bin-int/release/%.o: src/%.c
	$(CC) $^ -o $@ $(CFLAGS)

F:
	make clear || make

deploy:
	sudo cp bin/liblzy.a /usr/local/lib

clear:
	rm bin/test bin-int/release/*.o bin-int/debug/*.o bin/liblzy.a
