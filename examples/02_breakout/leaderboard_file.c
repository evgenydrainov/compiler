ScoreFile :: struct
{
	scores: [LEADERBOARD_SIZE]int;
	names: [LEADERBOARD_SIZE][NAME_CAPACITY]u8;
};

leaderboard_load :: proc(board: *Leaderboard) -> bool
{
	size: i32;
	data := LoadFileData("score.dat", &size);
	if data == null
	{
		return false;
	}

	defer UnloadFileData(data);

	file := cast(*ScoreFile)data;
	if size != sizeof(*file)
	{
		return false;
	}

	board.scores = file.scores;
	board.names = file.names;
	board.count = LEADERBOARD_SIZE;

	return true;
}

leaderboard_save :: proc(board: *Leaderboard) -> bool
{
	file: ScoreFile;
	file.scores = board.scores;
	file.names = board.names;

	return SaveFileData("score.dat", &file, sizeof(file));
}
