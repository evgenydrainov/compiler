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

// Color, 4 components, R8G8B8A8 (32bit)
Color :: struct {
    r : u8;        // Color red value
    g : u8;        // Color green value
    b : u8;        // Color blue value
    a : u8;        // Color alpha value
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

//----------------------------------------------------------------------------------
// Enumerators Definition
//----------------------------------------------------------------------------------
// System/Window config flags
// NOTE: Every bit registers one state (use it with bit masks)
// By default all flags are set to 0
FLAG_VSYNC_HINT         :: 0x00000040;   // Set to try enabling V-Sync on GPU
FLAG_FULLSCREEN_MODE    :: 0x00000002;   // Set to run program in fullscreen
FLAG_WINDOW_RESIZABLE   :: 0x00000004;   // Set to allow resizable window
FLAG_WINDOW_UNDECORATED :: 0x00000008;   // Set to disable window decoration (frame and buttons)
FLAG_WINDOW_HIDDEN      :: 0x00000080;   // Set to hide window
FLAG_WINDOW_MINIMIZED   :: 0x00000200;   // Set to minimize window (iconify)
FLAG_WINDOW_MAXIMIZED   :: 0x00000400;   // Set to maximize window (expanded to monitor)
FLAG_WINDOW_UNFOCUSED   :: 0x00000800;   // Set to window non focused
FLAG_WINDOW_TOPMOST     :: 0x00001000;   // Set to window always on top
FLAG_WINDOW_ALWAYS_RUN  :: 0x00000100;   // Set to allow windows running while minimized
FLAG_WINDOW_TRANSPARENT :: 0x00000010;   // Set to allow transparent framebuffer
FLAG_WINDOW_HIGHDPI     :: 0x00002000;   // Set to support HighDPI
FLAG_WINDOW_MOUSE_PASSTHROUGH :: 0x00004000; // Set to support mouse passthrough, only supported when FLAG_WINDOW_UNDECORATED
FLAG_BORDERLESS_WINDOWED_MODE :: 0x00008000; // Set to run program in borderless windowed mode
FLAG_MSAA_4X_HINT       :: 0x00000020;   // Set to try enabling MSAA 4X
FLAG_INTERLACED_HINT    :: 0x00010000;   // Set to try enabling interlaced video format (for V3D)

// Keyboard keys (US keyboard layout)
// NOTE: Use GetKeyPressed() to allow redefining required keys for alternative layouts
KEY_NULL            :: 0;        // Key: NULL, used for no key pressed
// Alphanumeric keys
KEY_APOSTROPHE      :: 39;       // Key: '
KEY_COMMA           :: 44;       // Key: ,
KEY_MINUS           :: 45;       // Key: -
KEY_PERIOD          :: 46;       // Key: .
KEY_SLASH           :: 47;       // Key: /
KEY_ZERO            :: 48;       // Key: 0
KEY_ONE             :: 49;       // Key: 1
KEY_TWO             :: 50;       // Key: 2
KEY_THREE           :: 51;       // Key: 3
KEY_FOUR            :: 52;       // Key: 4
KEY_FIVE            :: 53;       // Key: 5
KEY_SIX             :: 54;       // Key: 6
KEY_SEVEN           :: 55;       // Key: 7
KEY_EIGHT           :: 56;       // Key: 8
KEY_NINE            :: 57;       // Key: 9
KEY_SEMICOLON       :: 59;       // Key: ;
KEY_EQUAL           :: 61;       // Key: =
KEY_A               :: 65;       // Key: A | a
KEY_B               :: 66;       // Key: B | b
KEY_C               :: 67;       // Key: C | c
KEY_D               :: 68;       // Key: D | d
KEY_E               :: 69;       // Key: E | e
KEY_F               :: 70;       // Key: F | f
KEY_G               :: 71;       // Key: G | g
KEY_H               :: 72;       // Key: H | h
KEY_I               :: 73;       // Key: I | i
KEY_J               :: 74;       // Key: J | j
KEY_K               :: 75;       // Key: K | k
KEY_L               :: 76;       // Key: L | l
KEY_M               :: 77;       // Key: M | m
KEY_N               :: 78;       // Key: N | n
KEY_O               :: 79;       // Key: O | o
KEY_P               :: 80;       // Key: P | p
KEY_Q               :: 81;       // Key: Q | q
KEY_R               :: 82;       // Key: R | r
KEY_S               :: 83;       // Key: S | s
KEY_T               :: 84;       // Key: T | t
KEY_U               :: 85;       // Key: U | u
KEY_V               :: 86;       // Key: V | v
KEY_W               :: 87;       // Key: W | w
KEY_X               :: 88;       // Key: X | x
KEY_Y               :: 89;       // Key: Y | y
KEY_Z               :: 90;       // Key: Z | z
KEY_LEFT_BRACKET    :: 91;       // Key: [
KEY_BACKSLASH       :: 92;       // Key: '\'
KEY_RIGHT_BRACKET   :: 93;       // Key: ]
KEY_GRAVE           :: 96;       // Key: `
// Function keys
KEY_SPACE           :: 32;       // Key: Space
KEY_ESCAPE          :: 256;      // Key: Esc
KEY_ENTER           :: 257;      // Key: Enter
KEY_TAB             :: 258;      // Key: Tab
KEY_BACKSPACE       :: 259;      // Key: Backspace
KEY_INSERT          :: 260;      // Key: Ins
KEY_DELETE          :: 261;      // Key: Del
KEY_RIGHT           :: 262;      // Key: Cursor right
KEY_LEFT            :: 263;      // Key: Cursor left
KEY_DOWN            :: 264;      // Key: Cursor down
KEY_UP              :: 265;      // Key: Cursor up
KEY_PAGE_UP         :: 266;      // Key: Page up
KEY_PAGE_DOWN       :: 267;      // Key: Page down
KEY_HOME            :: 268;      // Key: Home
KEY_END             :: 269;      // Key: End
KEY_CAPS_LOCK       :: 280;      // Key: Caps lock
KEY_SCROLL_LOCK     :: 281;      // Key: Scroll down
KEY_NUM_LOCK        :: 282;      // Key: Num lock
KEY_PRINT_SCREEN    :: 283;      // Key: Print screen
KEY_PAUSE           :: 284;      // Key: Pause
KEY_F1              :: 290;      // Key: F1
KEY_F2              :: 291;      // Key: F2
KEY_F3              :: 292;      // Key: F3
KEY_F4              :: 293;      // Key: F4
KEY_F5              :: 294;      // Key: F5
KEY_F6              :: 295;      // Key: F6
KEY_F7              :: 296;      // Key: F7
KEY_F8              :: 297;      // Key: F8
KEY_F9              :: 298;      // Key: F9
KEY_F10             :: 299;      // Key: F10
KEY_F11             :: 300;      // Key: F11
KEY_F12             :: 301;      // Key: F12
KEY_LEFT_SHIFT      :: 340;      // Key: Shift left
KEY_LEFT_CONTROL    :: 341;      // Key: Control left
KEY_LEFT_ALT        :: 342;      // Key: Alt left
KEY_LEFT_SUPER      :: 343;      // Key: Super left
KEY_RIGHT_SHIFT     :: 344;      // Key: Shift right
KEY_RIGHT_CONTROL   :: 345;      // Key: Control right
KEY_RIGHT_ALT       :: 346;      // Key: Alt right
KEY_RIGHT_SUPER     :: 347;      // Key: Super right
KEY_KB_MENU         :: 348;      // Key: KB menu
// Keypad keys
KEY_KP_0            :: 320;      // Key: Keypad 0
KEY_KP_1            :: 321;      // Key: Keypad 1
KEY_KP_2            :: 322;      // Key: Keypad 2
KEY_KP_3            :: 323;      // Key: Keypad 3
KEY_KP_4            :: 324;      // Key: Keypad 4
KEY_KP_5            :: 325;      // Key: Keypad 5
KEY_KP_6            :: 326;      // Key: Keypad 6
KEY_KP_7            :: 327;      // Key: Keypad 7
KEY_KP_8            :: 328;      // Key: Keypad 8
KEY_KP_9            :: 329;      // Key: Keypad 9
KEY_KP_DECIMAL      :: 330;      // Key: Keypad .
KEY_KP_DIVIDE       :: 331;      // Key: Keypad /
KEY_KP_MULTIPLY     :: 332;      // Key: Keypad *
KEY_KP_SUBTRACT     :: 333;      // Key: Keypad -
KEY_KP_ADD          :: 334;      // Key: Keypad +
KEY_KP_ENTER        :: 335;      // Key: Keypad Enter
KEY_KP_EQUAL        :: 336;      // Key: Keypad =
// Android key buttons
KEY_BACK            :: 4;        // Key: Android back button
KEY_MENU            :: 5;        // Key: Android menu button
KEY_VOLUME_UP       :: 24;       // Key: Android volume up button
KEY_VOLUME_DOWN     :: 25;       // Key: Android volume down button

//------------------------------------------------------------------------------------
// Window and Graphics Device Functions (Module: core)
//------------------------------------------------------------------------------------

// Window-related functions
InitWindow        :: proc(width: i32, height: i32, title: *u8) #foreign; // Initialize window and OpenGL context
CloseWindow       :: proc()                                    #foreign; // Close window and unload OpenGL context
WindowShouldClose :: proc() -> bool                            #foreign; // Check if application should close (KEY_ESCAPE pressed or windows close icon clicked)
ToggleFullscreen  :: proc()                                    #foreign; // Toggle window state: fullscreen/windowed, resizes monitor to match window resolution
ToggleBorderlessWindowed :: proc()                             #foreign; // Toggle window state: borderless windowed, resizes window to match monitor resolution
GetScreenWidth    :: proc() -> i32                             #foreign; // Get current screen width
GetScreenHeight   :: proc() -> i32                             #foreign; // Get current screen height

// Drawing-related functions
ClearBackground :: proc(color: Color)      #foreign; // Set background color (framebuffer clear color)
BeginDrawing    :: proc()                  #foreign; // Setup canvas (framebuffer) to start drawing
EndDrawing      :: proc()                  #foreign; // End canvas drawing and swap buffers (double buffering)
BeginMode2D     :: proc(camera: Camera2D)  #foreign; // Begin 2D mode with custom camera (2D)
EndMode2D       :: proc()                  #foreign; // Ends 2D mode with custom camera

// Timing-related functions
SetTargetFPS :: proc(fps: i32) #foreign; // Set target FPS (maximum)
GetFrameTime :: proc() -> f32  #foreign; // Get time in seconds for last frame drawn (delta time)
GetTime      :: proc() -> f64  #foreign; // Get elapsed time in seconds since InitWindow()
GetFPS       :: proc() -> i32  #foreign; // Get current FPS

// Misc. functions
SetConfigFlags :: proc(flags: u32) #foreign; // Setup init configuration flags (view FLAGS)

//------------------------------------------------------------------------------------
// Input Handling Functions (Module: core)
//------------------------------------------------------------------------------------

// Input-related functions: keyboard
IsKeyPressed       :: proc(key: i32) -> bool #foreign; // Check if a key has been pressed once
IsKeyPressedRepeat :: proc(key: i32) -> bool #foreign; // Check if a key has been pressed again
IsKeyDown          :: proc(key: i32) -> bool #foreign; // Check if a key is being pressed
IsKeyReleased      :: proc(key: i32) -> bool #foreign; // Check if a key has been released once
IsKeyUp            :: proc(key: i32) -> bool #foreign; // Check if a key is NOT being pressed
SetExitKey         :: proc(key: i32)         #foreign; // Set a custom key to exit program (default is ESC)

// Basic shapes drawing functions
DrawPixel     :: proc(posX: i32, posY: i32, color: Color)                          #foreign; // Draw a pixel using geometry [Can be slow, use with care]
DrawRectangle :: proc(posX: i32, posY: i32, width: i32, height: i32, color: Color) #foreign; // Draw a color-filled rectangle

// Texture loading functions
// NOTE: These functions require GPU access
LoadTexture :: proc(fileName: *u8) -> Texture #foreign; // Load texture from file into GPU memory (VRAM)

// Texture drawing functions
DrawTexture    :: proc(texture: Texture, posX: i32, posY: i32, tint: i64)                 #foreign; // Draw a Texture2D
DrawTextureRec :: proc(texture: Texture, source: Rectangle, position: Vector2, tint: i64) #foreign; // Draw a part of a texture defined by a rectangle

// Color/pixel related functions
GetColor :: proc(hexValue: u32) -> Color #foreign; // Get Color structure from hexadecimal value

// Text drawing functions
DrawFPS  :: proc(posX: i32, posY: i32)                                         #foreign; // Draw current FPS
DrawText :: proc(text: *u8, posX: i32, posY: i32, fontSize: i32, color: Color) #foreign; // Draw text (using default font)

// Text font info functions
MeasureText :: proc(text: *u8, fontSize: i32) -> i32 #foreign; // Measure string width for default font

// Text strings management functions (no UTF-8 strings, only byte chars)
// WARNING 1: Most of these functions use internal static buffers[], it's recommended to store returned data on user-side for re-use
// WARNING 2: Some functions allocate memory internally for the returned strings, those strings must be freed by user using MemFree()
TextFormat :: proc(text: *u8) -> *u8 #foreign #variadic; // Text formatting with variables (sprintf() style)
