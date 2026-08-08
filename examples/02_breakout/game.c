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
	switch game.state
	{
		case .title: title_draw(&game.title);
		case .world: world_draw(&game.world);
	}
}
