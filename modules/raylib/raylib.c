// Vector2, 2 components
Vector2 :: struct {
    x : f32;                // Vector x component
    y : f32;                // Vector y component
};

// Vector3, 3 components
Vector3 :: struct {
    x : f32;                // Vector x component
    y : f32;                // Vector y component
    z : f32;                // Vector z component
};

// Vector4, 4 components
Vector4 :: struct {
    x : f32;                // Vector x component
    y : f32;                // Vector y component
    z : f32;                // Vector z component
    w : f32;                // Vector w component
};

// Rectangle, 4 components
Rectangle :: struct {
    x      : f32;           // Rectangle top-left corner position x
    y      : f32;           // Rectangle top-left corner position y
    width  : f32;           // Rectangle width
    height : f32;           // Rectangle height
};

// Texture, tex data stored in GPU memory (VRAM)
Texture :: struct {
    id      : u32;            // OpenGL texture id
    width   : i32;            // Texture base width
    height  : i32;            // Texture base height
    mipmaps : i32;            // Mipmap levels, 1 by default
    format  : i32;            // Data format (PixelFormat type)
};

// Camera2D, defines position/orientation in 2d space
Camera2D :: struct {
    offset   : Vector2;     // Camera offset (screen space offset from window origin)
    target   : Vector2;     // Camera target (world space target point that is mapped to screen space offset)
    rotation : f32;         // Camera rotation in degrees (pivots around target)
    zoom     : f32;         // Camera zoom (scaling around target), must not be set to 0, set to 1.0f for no scale
};

//------------------------------------------------------------------------------------
// Window and Graphics Device Functions (Module: core)
//------------------------------------------------------------------------------------

// Window-related functions
InitWindow        :: proc(width: i32, height: i32, title: *u8) #foreign; // Initialize window and OpenGL context
CloseWindow       :: proc()                                    #foreign; // Close window and unload OpenGL context
WindowShouldClose :: proc() -> bool                            #foreign; // Check if application should close (KEY_ESCAPE pressed or windows close icon clicked)

// Drawing-related functions
ClearBackground :: proc(color: i64)        #foreign; // Set background color (framebuffer clear color)
BeginDrawing    :: proc()                  #foreign; // Setup canvas (framebuffer) to start drawing
EndDrawing      :: proc()                  #foreign; // End canvas drawing and swap buffers (double buffering)
BeginMode2D     :: proc(camera: *Camera2D) #foreign; // Begin 2D mode with custom camera (2D)
EndMode2D       :: proc()                  #foreign; // Ends 2D mode with custom camera

// Timing-related functions
SetTargetFPS :: proc(fps: i32) #foreign; // Set target FPS (maximum)
GetFrameTime :: proc() -> f32  #foreign; // Get time in seconds for last frame drawn (delta time)
GetTime      :: proc() -> f64  #foreign; // Get elapsed time in seconds since InitWindow()
GetFPS       :: proc() -> i32  #foreign; // Get current FPS

//------------------------------------------------------------------------------------
// Input Handling Functions (Module: core)
//------------------------------------------------------------------------------------

// Input-related functions: keyboard
IsKeyPressed       :: proc(key: i32) -> bool #foreign; // Check if a key has been pressed once
IsKeyPressedRepeat :: proc(key: i32) -> bool #foreign; // Check if a key has been pressed again
IsKeyDown          :: proc(key: i32) -> bool #foreign; // Check if a key is being pressed
IsKeyReleased      :: proc(key: i32) -> bool #foreign; // Check if a key has been released once
IsKeyUp            :: proc(key: i32) -> bool #foreign; // Check if a key is NOT being pressed

// Basic shapes drawing functions
DrawPixel :: proc(posX: i32, posY: i32, color: i64) #foreign; // Draw a pixel using geometry [Can be slow, use with care]

// Texture loading functions
// NOTE: These functions require GPU access
LoadTexture :: proc(texture: *Texture, fileName: *u8) #foreign; // Load texture from file into GPU memory (VRAM)

// Texture drawing functions
DrawTexture    :: proc(texture: *Texture, posX: i32, posY: i32, tint: i64)                  #foreign; // Draw a Texture2D
DrawTextureRec :: proc(texture: *Texture, source: *Rectangle, position: Vector2, tint: i64) #foreign; // Draw a part of a texture defined by a rectangle

// Text drawing functions
DrawFPS  :: proc(posX: i32, posY: i32)                                       #foreign; // Draw current FPS
DrawText :: proc(text: *u8, posX: i32, posY: i32, fontSize: i32, color: i64) #foreign; // Draw text (using default font)

// Text font info functions
MeasureText :: proc(text: *u8, fontSize: i32) -> i32 #foreign; // Measure string width for default font
