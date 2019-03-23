
# PA8

### Build &amp; Run

    mkdir build
    cd build
    cp ../makefile .
    make
    ./PA8 <file>

### File Structure

- include: .h files
- src: .cpp files
- data:
	- models: .obj meshes
	- objects: .json object properties
	- shaders: .v/.f GLSL shaders
	- textures: images for meshes

### Libraries

- SDL2
- GLEW
- GLM
- Assimp
- Bullet
- stb_image (source provided)
- Dear ImGui (source provided)
   - uses stb_truetype, stb_textedit, stb_rect_pack

### Keys

Key 				| Bind
--------------------|-----------------------------------------
Esc					| exit
Click + Drag		| orbit camera
R 					| Reset object positions &amp; velocities
Space 				| Change selected object

### Extra Credit

- Mesh colliders (used for the board)
