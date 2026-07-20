BallState :: enum
{
	StickToPaddle;
	Move;
};

Entity :: struct
{
	x: f32;
	y: f32;

	hspeed: f32;
	vspeed: f32;

	width: f32;
	height: f32;

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
	world.paddle.x = cast(f32)(GAME_WIDTH/2);
	world.paddle.y = cast(f32)(GAME_HEIGHT/10*9);
	world.paddle.width = 40.0;
	world.paddle.height = 10.0;

	world.ball.width = 10.0;
	world.ball.height = 10.0;

	world.numBricks = 12;

	foreach brickIndex : 0..<world.numBricks
	{
		brick := &world.bricks[brickIndex];

		brick.width = 40.0;
		brick.height = 10.0;
		brick.x = 35.0 + (brick.width  + 10.0) * cast(f32)(brickIndex%6);
		brick.y = 10.0 + (brick.height + 10.0) * cast(f32)(brickIndex/6);
	}
}

WorldUpdate :: proc(world: *World, delta: f32)
{
	paddle := &world.paddle;
	ball := &world.ball;

	if IsKeyDown(KEY_RIGHT)
	{
		paddle.x += 4.0 * delta;
	}
	if IsKeyDown(KEY_LEFT)
	{
		paddle.x -= 4.0 * delta;
	}

	if ball.ballState == .StickToPaddle
	{
		ball.x = paddle.x;
		ball.y = paddle.y - (paddle.height + ball.height)*0.5;

		if IsKeyPressed(KEY_SPACE)
		{
			ball.hspeed = 2.0;
			ball.vspeed = -2.0;
			ball.ballState = .Move;
		}
	}
	else if ball.ballState == .Move
	{
		ball.x += ball.hspeed * delta;
		ball.y += ball.vspeed * delta;

		if ball.x + ball.width*0.5 >= cast(f32)GAME_WIDTH
		{
			ball.hspeed = -ball.hspeed;
		}
		if ball.x - ball.width*0.5 < 0.0
		{
			ball.hspeed = -ball.hspeed;
		}

		if ball.y + ball.height*0.5 >= cast(f32)GAME_HEIGHT
		{
			ball.vspeed = -ball.vspeed;
		}
		if ball.y - ball.height*0.5 < 0.0
		{
			ball.vspeed = -ball.vspeed;
		}

		if EntitiesCollide(ball, paddle)
		{
			ball.vspeed = -ball.vspeed;
		}

		foreach brickIndex : 0..<world.numBricks
		{
			brick := &world.bricks[brickIndex];

			if EntitiesCollide(ball, brick)
			{
				ball.vspeed = -ball.vspeed;
				//brick.width = 0;
				//brick.height = 0;
			}
		}
	}
}

DrawEntity :: proc(entity: *Entity, color: int)
{
	DrawRectangle(cast(i32)(entity.x - entity.width*0.5),
				  cast(i32)(entity.y - entity.height*0.5),
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
	return RectVsRect(entity1.x - entity1.width*0.5,
					  entity1.y - entity1.height*0.5,
					  entity1.width,
					  entity1.height,
					  entity2.x - entity2.width*0.5,
					  entity2.y - entity2.height*0.5,
					  entity2.width,
					  entity2.height);
}

RectVsRect :: proc(x1: f32, y1: f32, width1: f32, height1: f32,
				   x2: f32, y2: f32, width2: f32, height2: f32) -> bool
{
	return (x1 < x2+width2
			&& x1+width1 > x2
			&& y1 < y2+height2
			&& y1+height1 > y2);
}
