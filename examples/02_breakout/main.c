#include "../../modules/builtin.c"
#include "../../modules/raylib/raylib.c"

GAME_WIDTH  :: 320;
GAME_HEIGHT :: 240;

GameState :: enum
{
	Title;
	Gameplay;
};

Game :: struct
{
	state: GameState;
};

GameUpdate :: proc(game: *Game)
{
	if game.state == GameState.Title
	{

	}
}

GameRender :: proc(game: *Game)
{
	if game.state == GameState.Title
	{
		x := GAME_WIDTH/2;
		y := GAME_HEIGHT/2;
		
		draw_text_centered("BREAKOUT"c, x, y, 10, 0xffffffff);
		y += 20;

		draw_text_centered("PRESS ENTER TO START"c, x, y, 10, 0xffffffff);
		y += 20;
	}
}

main :: proc() -> i64
{
	InitWindow(640, 480, "breakout"c);
	SetTargetFPS(60);

	game: Game;

	while !WindowShouldClose()
	{
		GameUpdate(&game);

		BeginDrawing();

		ClearBackground(0);

		camera: Camera2D;
		camera.zoom = int64_to_float32(2);

		BeginMode2D(&camera);

		GameRender(&game);

		EndMode2D();

		EndDrawing();
	}

	CloseWindow();

	return 0;
}

draw_text_centered :: proc(text: *u8,
						   x: int, y: int,
						   fontSize: i32, color: int)
{
	textWidth := MeasureText(text, fontSize);

	DrawText(text,
			 cast(i32)(x - textWidth/2), cast(i32)y,
			 fontSize, color);
}
