GAME_WIDTH  :: 320;
GAME_HEIGHT :: 240;

GameState :: enum
{
	title;
	world;
};

Game :: struct
{
	state: GameState;
	title: Title;
	world: World;

	// only kept while the game is running
	high_score: int;
};

game_update :: proc(game: *Game, delta: f32)
{
	switch game.state
	{
		case .title: title_update(&game.title, game);
		case .world: world_update(&game.world, game, delta);
	}

	if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
		|| IsKeyPressed(KEY_F11)
	{
		ToggleBorderlessWindowed();
	}
}

game_draw :: proc(game: *Game)
{
	// draw game background
	DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, GetColor(0x151515ff));

	switch game.state
	{
		case .title: title_draw(&game.title, game);
		case .world: world_draw(&game.world);
	}
}
