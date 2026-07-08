TitleScreen :: struct
{
	dummy: int;
};

TitleScreenUpdate :: proc(title: *TitleScreen,
						  game: *Game)
{
	if IsKeyPressed(KEY_ENTER)
	{
		game.state = .World;
		WorldInit(&game.world);
	}
}

TitleScreenDraw :: proc(title: *TitleScreen)
{
	x := GAME_WIDTH/2;
	y := GAME_HEIGHT/2;
		
	draw_text_centered("BREAKOUT"c, x, y, 10, 0xffffffff);
	y += 20;

	draw_text_centered("PRESS ENTER TO START"c, x, y, 10, 0xffffffff);
	y += 20;
}
