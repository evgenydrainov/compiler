#include "../../modules/builtin.c"
#include "../../modules/raylib/raylib.c"

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
		DrawText("BREAKOUT"c, 140, 120, 10, 0xffffffff);

		DrawText("PRESS ENTER TO START"c, 100, 140, 10, 0xffffffff);
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
