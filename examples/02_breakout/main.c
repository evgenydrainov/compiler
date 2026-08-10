#import "raylib/raylib.c"

#include "title.c"
#include "world.c"
#include "game.c"
#include "util.c"

WINDOW_SCALE :: 2;

setup_screen_camera :: proc(camera: *Camera2D)
{
	screen_width  := cast(f32)GetScreenWidth();
	screen_height := cast(f32)GetScreenHeight();

	xscale := screen_width  / cast(f32)GAME_WIDTH;
	yscale := screen_height / cast(f32)GAME_HEIGHT;

	scale := fminf(xscale, yscale);

	scaled_width  := cast(f32)GAME_WIDTH  * scale;
	scaled_height := cast(f32)GAME_HEIGHT * scale;

	camera.offset.x = (screen_width  - scaled_width)  * 0.5;
	camera.offset.y = (screen_height - scaled_height) * 0.5;
	camera.zoom = scale;
}

main :: proc() -> int
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(GAME_WIDTH*WINDOW_SCALE, GAME_HEIGHT*WINDOW_SCALE, "BREAKOUT"c);
	SetExitKey(KEY_NULL);

	game: Game;

	while !WindowShouldClose()
	{
		delta := fminf(GetFrameTime()*60.0, 2.0);
		
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
