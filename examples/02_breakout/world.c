PADDLE_WIDTH :: 40;
PADDLE_HEIGHT :: 10;

Paddle :: struct
{
	x: int;
	y: int;
};

World :: struct
{
	paddle: Paddle;
};

WorldInit :: proc(world: *World)
{
	world.paddle.x = GAME_WIDTH/2;
	world.paddle.y = GAME_HEIGHT/10*9;
}

WorldUpdate :: proc(world: *World)
{
	if IsKeyDown(KEY_RIGHT)
	{
		world.paddle.x += 4;
	}
	if IsKeyDown(KEY_LEFT)
	{
		world.paddle.x -= 4;
	}
}

WorldDraw :: proc(world: *World)
{
	DrawRectangle(cast(i32)(world.paddle.x-PADDLE_WIDTH/2), cast(i32)(world.paddle.y-PADDLE_HEIGHT/2),
				  PADDLE_WIDTH, PADDLE_HEIGHT,
				  0xffffffff);
}
