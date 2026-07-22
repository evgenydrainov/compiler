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

GameUpdate :: proc(game: *Game, delta: f32)
{
	if game.state == .TitleScreen
	{
		TitleScreenUpdate(&game.title, game);
	}
	else if game.state == .World
	{
		WorldUpdate(&game.world, delta);
	}

	if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
		|| IsKeyPressed(KEY_F11)
	{
		ToggleBorderlessWindowed();
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

SetupScreenCamera :: proc(camera: *Camera2D)
{
	screenWidth  := cast(f32)GetScreenWidth();
	screenHeight := cast(f32)GetScreenHeight();

	xscale := screenWidth  / cast(f32)GAME_WIDTH;
	yscale := screenHeight / cast(f32)GAME_HEIGHT;

	scale := fminf(xscale, yscale);

	scaledWidth  := cast(f32)GAME_WIDTH  * scale;
	scaledHeight := cast(f32)GAME_HEIGHT * scale;

	camera.offset.x = (screenWidth  - scaledWidth)  * 0.5;
	camera.offset.y = (screenHeight - scaledHeight) * 0.5;
	camera.zoom = scale;
}

main :: proc() -> int
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(GAME_WIDTH*WINDOW_SCALE, GAME_HEIGHT*WINDOW_SCALE, "breakout"c);
	SetExitKey(KEY_NULL);

	game: Game;

	while !WindowShouldClose()
	{
		delta := GetFrameTime()*60.0;
		
		GameUpdate(&game, delta);

		BeginDrawing();

		ClearBackground(0);

		camera: Camera2D;
		SetupScreenCamera(&camera);

		BeginMode2D(&camera);

		GameDraw(&game);

		EndMode2D();

		DrawFPS(0, 0);

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
