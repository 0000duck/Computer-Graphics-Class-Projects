
CC=g++
LIBS=-lSDL2 -lSDL2_mixer -lGLEW -lGL -lassimp -lBulletDynamics -lBulletSoftBody -lBulletCollision -lLinearMath -pthread

CXXFLAGS=-O2 -Wall -std=c++0x
O_FILES=main.o camera.o engine.o graphics.o shader.o window.o imgui.o imgui_draw.o imgui_impl.o scene.o stb_image.o world.o object.o text.o sound.o
INCLUDES=-I../include -I../deps -I/usr/include/bullet/

all: $(O_FILES)
	$(CC) $(CXXFLAGS) -o PA10 $(O_FILES) $(LIBS)

main.o: ../src/main.cpp
	$(CC) $(CXXFLAGS) -c ../src/main.cpp -o main.o $(INCLUDES)

sound.o: ../src/sound.cpp
	$(CC) $(CXXFLAGS) -c ../src/sound.cpp -o sound.o $(INCLUDES)

camera.o: ../src/camera.cpp
	$(CC) $(CXXFLAGS) -c ../src/camera.cpp -o camera.o $(INCLUDES)

engine.o: ../src/engine.cpp
	$(CC) $(CXXFLAGS) -c ../src/engine.cpp -o engine.o $(INCLUDES)

graphics.o: ../src/graphics.cpp
	$(CC) $(CXXFLAGS) -c ../src/graphics.cpp -o graphics.o $(INCLUDES)

shader.o: ../src/shader.cpp
	$(CC) $(CXXFLAGS) -c ../src/shader.cpp -o shader.o $(INCLUDES)

window.o: ../src/window.cpp
	$(CC) $(CXXFLAGS) -c ../src/window.cpp -o window.o $(INCLUDES)

imgui_impl.o: ../src/imgui_impl.cpp
	$(CC) $(CXXFLAGS) -c ../src/imgui_impl.cpp -o imgui_impl.o $(INCLUDES)

imgui.o: ../deps/imgui.cpp 
	$(CC) $(CXXFLAGS) -c ../deps/imgui.cpp -o imgui.o $(INCLUDES)

imgui_draw.o: ../deps/imgui_draw.cpp 
	$(CC) $(CXXFLAGS) -c ../deps/imgui_draw.cpp -Wno-maybe-uninitialized -o imgui_draw.o $(INCLUDES)

scene.o: ../src/scene.cpp
	$(CC) $(CXXFLAGS) -c ../src/scene.cpp -o scene.o $(INCLUDES)		

world.o: ../src/world.cpp
	$(CC) $(CXXFLAGS) -c ../src/world.cpp -o world.o $(INCLUDES)		

object.o: ../src/object.cpp
	$(CC) $(CXXFLAGS) -c ../src/object.cpp -o object.o $(INCLUDES)		

stb_image.o: ../src/stb_image_impl.cpp
	$(CC) $(CXXFLAGS) -c ../src/stb_image_impl.cpp -o stb_image.o $(INCLUDES)	

text.o: ../src/text.cpp
	$(CC) $(CXXFLAGS) -c ../src/text.cpp -o text.o $(INCLUDES)		

clean:
	-@if rm *.o PA10>/dev/null || true; then echo "Main Removed"; else echo "No Main"; fi
