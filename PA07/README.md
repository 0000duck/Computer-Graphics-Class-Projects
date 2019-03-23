# Solar System Instruction Manual

#### Max Slater, Grant Hooks, RJ Lowry

### Build & Run
From PA7 directory:

    mkdir build
    cd build
    cp ../makefile .
    make
    ./PA7

### Menu

Menu Item | Functionality | Initial State
----------|---------------|--------------
FOV Slider | Adjusts the camera’s field of view | 50
Camera Type | Switch between the orbit camera, free camera, and tracking camera | Orbit
Free Camera: Speed | Change the movement speed of the free camera | 5
Tracking Camera: Track | Choose which planet to track with the tracking camera | Earth
Draw Orbit Paths | Renders a dotted line tracing the orbit of the planets and moons | On
Time Scale Slider | Adjusts the speed at which the solar system is simulated | 100
Use Time Scale | Toggles the use of time scale and real time simulation | On
Distance Scale Slider | Adjusts the scaled distance of the planets | 0.05
Diameter Scale Slider | Adjusts the scaled diameter of the planets | 25
Use Distance Scales | Toggles the use of realistically scaled distances rather than the optimized scaled view | Off

### Keys

#### Universal Controls
Key | Bind
----|-----
Esc | Exit

#### Camera Specific Controls
Camera State | Key | Bind
-------------|-----|-----
Orbit Camera | Click + Drag | Orbit Camera
Free Camera | Click + Drag | Rotate Camera
Free Camera | W, A, S, D |Move Camera

### Dependencies

- SDL2
- GLEW
- GLM
- Assimp
- picojson 	(source provided)
- stb_image 	(source provided)
- Dear ImGui 	(source provided)
	- uses stb_truetype, stb_textedit, stb_rect_pack

### UI

ImGui is used to provide multiple menu options to customize the simulation of the solar system.

### File Structure

- blender - .blend models created using blender
- data
    - models - .obj files for planet model (earth.obj) and rings
    - shaders - vertex and fragment shader code files
    - planets - JSON files containing planetary data including diameter, distance from sun, orbital period, rotation period, planet’s moons, and rings
    - textures - texture images
- deps - Dependencies - imgui for gui, stb_image for image loading, picojson for parsing json input
- include - .h files
- src - .cpp files

### Code Structure / Classes

- main - starts and runs engine
- engine - initializes system sand manages updates while running, handles keyboard and mouse events, starts solar system
- graphics_headers - describes basic types for use in graphics
- graphics - initializes camera, handles shaders, star cubemap, rendering contexts
- camera - describes 3 types of cameras based on a common interface
- window - initializes and manages SDL windowing and OpenGL context
- imgui_impl - provides setup and rendering functions for using ImGui with SDL and OpenGL
- solarsystem - describes a collection of planets
- planet - describes a planet, handles config loading, simulation, and rendering
- scene - describes a textured 3D mesh for rendering

![class graph](https://github.com/TheNumbat/cs480SlaterLowryHooks/raw/master/PA7/structure.png)

### Extra Credit

- Configuration Files (JSON)
- Live adjustment of simulation speed
- Draws planet orbit paths
    - Tilted orbit paths
- Option for actual data and scaled view
- Uranus rings
- Starry skybox
- Menu to adjust simulation and rendering options
    - Three types of interactive cameras
- Jupiter and Saturn moons
- Basic lighting from the sun

### Issues

- Wrangling OpenGL
    - Majority of bugs came in setting up / piping around OpenGL and shader state
- Optimizing nice-looking scale for viewing
    - This took a lot of tweaking to look reasonably good
- Making sure all transformations are correct/coherent
    - One or two bugs occurred regarding the correctness/ordering of transformations

### What We'd Do differently
- Need better integrated OpenGL contexts for multiple shaders and VAOs
    - Don’t try to keep OpenGL state within Graphics
- Engine / Graphics class separation is unnecessary and annoying
    - Could put all planet data in one JSON file; just make everything a moon of the sun
- The lighting model isn’t really Phong, it’s only diffuse 
    - Startup time is too long; we should use smaller textures

### Screenshots

![screenshot](https://github.com/TheNumbat/cs480SlaterLowryHooks/raw/master/PA7/screenshot_1.png)
![screenshot](https://github.com/TheNumbat/cs480SlaterLowryHooks/raw/master/PA7/screenshot_2.png)

