Title :: struct
{
	dummy: int;
};

title_update :: proc(title: *Title,
					 game: *Game)
{
	if IsKeyPressed(KEY_ENTER)
	{
		game.state = .world;
		world_init(&game.world);
	}
}

title_draw :: proc(title: *Title)
{
	x := GAME_WIDTH/2;
	y := 80;
		
	draw_text_centered("BREAKOUT"c, x, y, 20, 0xffffffff);
	y += 40;

	draw_text_centered("PRESS ENTER TO START"c, x, y, 20, 0xffffffff);
	y += 40;
}
