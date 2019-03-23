
# PA5

### Build &amp; Run

    mkdir build
    cd build
    cp ../makefile .
    make
    ./PA5 <file>

If a file is not provided, the default box will be loaded. The model may also be specified within the program.  
The file path is relative to the executable.  

### Libraries

- SDL2
- GLEW
- GLM
- Assimp
- Dear ImGui (no install, source provided)
   - uses stb_truetype, stb_textedit, stb_rect_pack

### Keys

Key 				| Bind
--------------------|-----------------------------------------
Esc					| exit
Click + Drag		| orbit camera

### UI

ImGui is used to provide an option to load/reload a model.  
