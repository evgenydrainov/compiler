BallState :: enum
{
	StickToPaddle;
	Move;
};

Entity :: struct
{
	x: int;
	y: int;
	hspeed: int;
	vspeed: int;
	width: int;
	height: int;
	ballState: BallState;
};

World :: struct
{
	paddle: Entity;
	ball: Entity;

	bricks: [12]Entity;
	numBricks: int;
};

WorldInit :: proc(world: *World)
{
	world.paddle.x = GAME_WIDTH/2;
	world.paddle.y = GAME_HEIGHT/10*9;
	world.paddle.width = 40;
	world.paddle.height = 10;

	world.ball.width = 10;
	world.ball.height = 10;

	world.numBricks = 12;

	foreach brickIndex : 0..<world.numBricks
	{
		brick := &world.bricks[brickIndex];

		brick.width = 40;
		brick.height = 10;
		brick.x = 35 + (brick.width  + 10) * (brickIndex%6);
		brick.y = 10 + (brick.height + 10) * (brickIndex/6);
	}
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

	if world.ball.ballState == .StickToPaddle
	{
		world.ball.x = world.paddle.x;
		world.ball.y = world.paddle.y - world.paddle.height/2 - world.ball.height/2;

		if IsKeyPressed(KEY_SPACE)
		{
			world.ball.hspeed = 2;
			world.ball.vspeed = -2;
			world.ball.ballState = .Move;
		}
	}
	else if world.ball.ballState == .Move
	{
		world.ball.x += world.ball.hspeed;
		world.ball.y += world.ball.vspeed;

		if world.ball.x + world.ball.width/2 >= GAME_WIDTH
		{
			world.ball.hspeed = -world.ball.hspeed;
		}
		if world.ball.x - world.ball.width/2 < 0
		{
			world.ball.hspeed = -world.ball.hspeed;
		}

		if world.ball.y + world.ball.height/2 >= GAME_HEIGHT
		{
			world.ball.vspeed = -world.ball.vspeed;
		}
		if world.ball.y - world.ball.height/2 < 0
		{
			world.ball.vspeed = -world.ball.vspeed;
		}

		if EntitiesCollide(&world.ball, &world.paddle)
		{
			world.ball.vspeed = -world.ball.vspeed;
		}

		foreach brickIndex : 0..<world.numBricks
		{
			brick := &world.bricks[brickIndex];

			if EntitiesCollide(&world.ball, brick)
			{
				world.ball.vspeed = -world.ball.vspeed;
				//brick.width = 0;
				//brick.height = 0;
			}
		}
	}
}

DrawEntity :: proc(entity: *Entity, color: int)
{
	DrawRectangle(cast(i32)(entity.x - entity.width/2),
				  cast(i32)(entity.y - entity.height/2),
				  cast(i32)entity.width,
				  cast(i32)entity.height,
				  color);
}

WorldDraw :: proc(world: *World)
{
	DrawEntity(&world.paddle, 0xffffffff);

	foreach brickIndex : 0..<world.numBricks
	{
		brick := &world.bricks[brickIndex];
		DrawEntity(brick, 0xffffffff);
	}

	DrawEntity(&world.ball, 0xffffffff);
}

EntitiesCollide :: proc(entity1: *Entity, entity2: *Entity) -> bool
{
	return RectVsRect(entity1.x - entity1.width/2,
					  entity1.y - entity1.height/2,
					  entity1.width,
					  entity1.height,
					  entity2.x - entity2.width/2,
					  entity2.y - entity2.height/2,
					  entity2.width,
					  entity2.height);
}

RectVsRect :: proc(x1: int, y1: int, width1: int, height1: int,
				   x2: int, y2: int, width2: int, height2: int) -> bool
{
	return (x1 < x2+width2
			&& x1+width1 > x2
			&& y1 < y2+height2
			&& y1+height1 > y2);
}
