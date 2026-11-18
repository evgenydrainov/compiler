Title :: struct
{
};

title_update :: proc(title: *Title,
					 game: *Game)
{
	if is_confirm_pressed()
	{
		game.state = .world;
		world_init(&game.world);
	}
	else if IsKeyPressed(KEY_L)
	{
		game.state = .leaderboard;
	}
}

title_draw :: proc(title: *Title,
				   game: *Game)
{
	x := GAME_WIDTH/2;
	y := 50;

	draw_text_centered2("BREAKOUT", x, y, 20);
	y += 40;

	draw_text_centered2("PRESS ENTER TO START", x, y, 20);
	y += 50;

	draw_text_centered("ARROWS - MOVE", x, y, 10, color_from_u32(0xa0a0a0ff));
	y += 15;
	draw_text_centered("SPACE - LAUNCH", x, y, 10, color_from_u32(0xa0a0a0ff));
	y += 15;
	draw_text_centered("P - PAUSE", x, y, 10, color_from_u32(0xa0a0a0ff));
	y += 15;
	draw_text_centered2("L - LEADERBOARD", x, y, 10);
	y += 15;
}
