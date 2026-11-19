#import "raylib/raylib.c"

#include "title.c"
#include "leaderboard.c"
#include "world.c"
#include "game.c"
#include "util.c"

//#subsystem_windows;

WINDOW_SCALE :: 2;

mouse_x: f32;
mouse_y: f32;

dos_font_png :: #embed "dos_font.png";

setup_screen_camera :: proc(camera: *Camera2D)
{
	screen_width  := cast(f32)GetScreenWidth();
	screen_height := cast(f32)GetScreenHeight();

	xscale := screen_width  / GAME_WIDTH;
	yscale := screen_height / GAME_HEIGHT;

	scale := fminf(xscale, yscale);

	scaled_width  := GAME_WIDTH  * scale;
	scaled_height := GAME_HEIGHT * scale;

	camera.offset.x = (screen_width  - scaled_width)  * 0.5;
	camera.offset.y = (screen_height - scaled_height) * 0.5;
	camera.zoom = scale;
	
	mouse_x = (cast(f32)GetMouseX() - camera.offset.x)/scale;
	mouse_y = (cast(f32)GetMouseY() - camera.offset.y)/scale;
}

main :: proc() -> int
//WinMain :: proc(hInstance: *void, hPrevInstance: *void, lpCmdLine: *u8, nShowCmd: i32) -> int #foreign
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(GAME_WIDTH*WINDOW_SCALE, GAME_HEIGHT*WINDOW_SCALE, "BREAKOUT");
	SetExitKey(KEY_NULL);

	font = LoadImageFontFromMemory(".png", &dos_font_png[0], dos_font_png.count);

	game: Game;
	leaderboard_init(&game.board);

	while !WindowShouldClose()
	{
		delta := fminf(GetFrameTime()*60, 2);
		
		game_update(&game, delta);

		BeginDrawing();

		ClearBackground(GetColor(0x000000ff));

		camera: Camera2D;
		setup_screen_camera(&camera);

		BeginMode2D(camera);
		game_draw(&game);
		EndMode2D();

		DrawFPS(GetScreenWidth()-100, GetScreenHeight()-20);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}
