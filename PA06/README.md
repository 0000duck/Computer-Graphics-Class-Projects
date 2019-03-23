
# PA6

### Build &amp; Run

    mkdir build
    cd build
    cp ../makefile .
    make
    ./PA6 <file>

If a file is not provided, the default cube will be loaded. The model may also be specified within the program.  
The file path is relative to the executable, and textures will be loaded relative to the object files.

### Libraries

- SDL2
- GLEW
- GLM
- Assimp
- stb_image (no install, source provided)
- Dear ImGui (no install, source provided)
   - uses stb_truetype, stb_textedit, stb_rect_pack

### Keys

Key 				| Bind
--------------------|-----------------------------------------
Esc					| exit
Click + Drag		| orbit camera

### UI

ImGui is used to provide an option to load/reload a model.  
