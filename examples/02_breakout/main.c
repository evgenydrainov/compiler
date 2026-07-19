#import "raylib/raylib.c"

#include "title.c"
#include "world.c"

GAME_WIDTH  :: 320;
GAME_HEIGHT :: 240;

WINDOW_SCALE :: 2;

GameState :: enum
{
	TitleScreen;
	World;
};

Game :: struct
{
	state: GameState;
	title: TitleScreen;
	world: World;
};

GameUpdate :: proc(game: *Game)
{
	if game.state == .TitleScreen
	{
		TitleScreenUpdate(&game.title, game);
	}
	else if game.state == .World
	{
		WorldUpdate(&game.world);
	}
}

GameDraw :: proc(game: *Game)
{
	if game.state == .TitleScreen
	{
		TitleScreenDraw(&game.title);
	}
	else if game.state == .World
	{
		WorldDraw(&game.world);
	}
}

main :: proc() -> int
{
	InitWindow(GAME_WIDTH*WINDOW_SCALE, GAME_HEIGHT*WINDOW_SCALE, "breakout"c);
	SetTargetFPS(60);
	
	game: Game;

	while !WindowShouldClose()
	{
		GameUpdate(&game);

		BeginDrawing();

		ClearBackground(0);

		camera: Camera2D;
		camera.zoom = cast(f32)WINDOW_SCALE;

		BeginMode2D(&camera);

		GameDraw(&game);

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
