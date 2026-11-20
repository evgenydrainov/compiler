#link_library "gamelib.lib";

vec2 :: struct
{
	x, y: f32;
};

vec3 :: struct
{
	x, y, z: f32;
};

vec4 :: struct
{
	x, y, z, w: f32;
};

mat4 :: struct
{
	m: [4][4]f32;
};

Texture :: struct
{
	id: u32;
	width: i32;
	height: i32;
};

Vertex :: struct
{
	pos: vec3;
	texcoord: vec2;
	color: u32;
};

Rect :: struct
{
	x: i32;
	y: i32;
	width: i32;
	height: i32;
};

Key :: enum
{
	A :: 4;
	B :: 5;
	C :: 6;
	D :: 7;
	E :: 8;
	F :: 9;
	G :: 10;
	H :: 11;
	I :: 12;
	J :: 13;
	K :: 14;
	L :: 15;
	M :: 16;
	N :: 17;
	O :: 18;
	P :: 19;
	Q :: 20;
	R :: 21;
	S :: 22;
	T :: 23;
	U :: 24;
	V :: 25;
	W :: 26;
	X :: 27;
	Y :: 28;
	Z :: 29;

	_1 :: 30;
	_2 :: 31;
	_3 :: 32;
	_4 :: 33;
	_5 :: 34;
	_6 :: 35;
	_7 :: 36;
	_8 :: 37;
	_9 :: 38;
	_0 :: 39;

	RETURN :: 40;
	ESCAPE :: 41;
	BACKSPACE :: 42;
	TAB :: 43;
	SPACE :: 44;
	MINUS :: 45;
	EQUALS :: 46;
	LEFTBRACKET :: 47;
	RIGHTBRACKET :: 48;
	BACKSLASH :: 49;
	SEMICOLON :: 51;
	APOSTROPHE :: 52;
	GRAVE :: 53;
	COMMA :: 54;
	PERIOD :: 55;
	SLASH :: 56;
	CAPSLOCK :: 57;

	F1 :: 58;
	F2 :: 59;
	F3 :: 60;
	F4 :: 61;
	F5 :: 62;
	F6 :: 63;
	F7 :: 64;
	F8 :: 65;
	F9 :: 66;
	F10 :: 67;
	F11 :: 68;
	F12 :: 69;

	PRINTSCREEN :: 70;
	SCROLLLOCK :: 71;
	PAUSE :: 72;
	INSERT :: 73;
	HOME :: 74;
	PAGEUP :: 75;
	DELETE :: 76;
	END :: 77;
	PAGEDOWN :: 78;
	RIGHT :: 79;
	LEFT :: 80;
	DOWN :: 81;
	UP :: 82;

	LCTRL :: 224;
	LSHIFT :: 225;
	LALT :: 226;
	LGUI :: 227;
	RCTRL :: 228;
	RSHIFT :: 229;
	RALT :: 230;
	RGUI :: 231;
};

window_create :: proc(title: *u8, width: i32, height: i32) #foreign;

window_close :: proc() #foreign;

window_should_close :: proc() -> bool #foreign;

handle_events :: proc() #foreign;

swap_buffers :: proc() #foreign;

set_vsync :: proc(enable: bool) #foreign;
get_vsync :: proc() -> bool #foreign;

set_fullscreen :: proc(enable: bool) #foreign;
get_fullscreen :: proc() -> bool #foreign;

is_key_down :: proc(key: Key) -> bool #foreign;
is_key_pressed :: proc(key: Key) -> bool #foreign;
is_key_released :: proc(key: Key) -> bool #foreign;

begin_drawing :: proc() #foreign;
end_drawing :: proc() #foreign;

set_viewport :: proc(x: i32, y: i32, width: i32, height: i32) #foreign;

clear_color :: proc(color: vec4) #foreign;

load_texture :: proc(filepath: *u8) -> Texture #foreign;

draw_texture :: proc(texture: Texture, x: f32, y: f32) #foreign;

draw_texture2 :: proc(texture: Texture, source: Rect, x: f32, y: f32) #foreign;

draw_rectangle :: proc(x: i32, y: i32, width: i32, height: i32, color: vec4) #foreign;

draw_quad :: proc(texture: Texture, vertices: *Vertex) #foreign;

identity :: proc() -> mat4 #foreign;

ortho :: proc(left: f32, right: f32, bottom: f32, top: f32) -> mat4 #foreign;

color_to_u32 :: proc(color: vec4) -> u32 #foreign;

color_from_u32 :: proc(color: u32) -> vec4 #foreign;
