LEADERBOARD_SIZE :: 8;
NAME_CAPACITY :: 13; // Twelve printable characters and a zero terminator.

Leaderboard :: struct
{
	scores: [LEADERBOARD_SIZE]int;
	names: [LEADERBOARD_SIZE][NAME_CAPACITY]u8;
	count: int;

	name: [NAME_CAPACITY]u8;
	name_length: int;
};

leaderboard_seed_score :: proc(board: *Leaderboard, name: *u8, score: int)
{
	i := 0;
	while i < NAME_CAPACITY - 1 && name[i] != 0
	{
		board.name[i] = name[i];
		i += 1;
	}
	board.name[i] = 0;
	leaderboard_submit(board, score);
}

leaderboard_init :: proc(board: *Leaderboard)
{
	leaderboard_seed_score(board, "NOVA", 800);
	leaderboard_seed_score(board, "COMET", 600);
	leaderboard_seed_score(board, "PIXEL", 450);
	leaderboard_seed_score(board, "ORBIT", 300);
	leaderboard_seed_score(board, "EMBER", 200);
	leaderboard_seed_score(board, "PIP", 100);
	leaderboard_seed_score(board, "DOT", 50);
	leaderboard_seed_score(board, "ACE", 10);
	board.name[0] = 0;
}

leaderboard_submit :: proc(board: *Leaderboard, score: int)
{
	rank := 0;
	while rank < board.count && board.scores[rank] >= score
	{
		rank += 1;
	}

	if rank == LEADERBOARD_SIZE
	{
		return;
	}

	board.count = min_i64(board.count + 1, LEADERBOARD_SIZE);
	i := board.count - 1;
	while i > rank
	{
		board.scores[i] = board.scores[i - 1];
		board.names[i] = board.names[i - 1];
		i -= 1;
	}
	board.scores[rank] = score;
	board.names[rank] = board.name;
}

name_entry_update :: proc(game: *Game)
{
	board := &game.board;
	ch := GetCharPressed();
	while ch != 0
	{
		if ch >= 32 && ch <= 126 && board.name_length < NAME_CAPACITY - 1
			&& (ch != 32 || board.name_length > 0)
		{
			board.name[board.name_length] = cast(u8)ch;
			board.name_length += 1;
			board.name[board.name_length] = 0;
		}
		ch = GetCharPressed();
	}
	if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && board.name_length > 0
	{
		board.name_length -= 1;
		board.name[board.name_length] = 0;
	}
	if is_confirm_pressed() && board.name_length > 0
	{
		leaderboard_submit(board, game.world.score);
		game.state = .leaderboard;
	}
}

name_entry_draw :: proc(game: *Game)
{
	x := GAME_WIDTH*0.5;

	draw_text_centered2("ENTER YOUR NAME", x, 48, 2);
	draw_text_centered2(TextFormat("SCORE: %lld", game.world.score), x, 82, 1);
	draw_text_centered2(TextFormat("%s_", &game.board.name[0]), x, 110, 2);
	draw_text_centered2("UP TO 12 CHARACTERS", x, 153, 1);
	draw_text_centered2("PRESS ENTER TO ADD SCORE", x, 185, 1);
}

leaderboard_draw_row :: proc(board: *Leaderboard, i: int)
{
	y := 50.0 + cast(f32)i*18.0;

	draw_text1(TextFormat("%lld.", i + 1), 30, y, 1);
	draw_text1(&board.names[i][0], 57, y, 1);

	score := TextFormat("%lld", board.scores[i]);
	score_size := measure_text(score, 1);
	x := 290.0 - score_size.x;
	draw_text1(score, x, y, 1);
}

leaderboard_draw :: proc(board: *Leaderboard)
{
	draw_text_centered2("LEADERBOARD", GAME_WIDTH*0.5, 15, 2);
	for i in 0..<board.count
	{
		leaderboard_draw_row(board, i);
	}
	draw_text_centered2("PRESS ENTER FOR TITLE", GAME_WIDTH*0.5, 214, 1);
}
