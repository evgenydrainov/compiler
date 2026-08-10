BallState :: enum
{
	stick_to_paddle;
	move;
};

Entity :: struct
{
	x: f32;
	y: f32;

	hspeed: f32;
	vspeed: f32;

	width: f32;
	height: f32;

	ball_state: BallState;

	dead: bool;
};

World :: struct
{
	paddle: Entity;
	ball: Entity;

	bricks: [12]Entity;
	num_bricks: int;

	lives: int;
	level: int;
};

world_init :: proc(world: *World)
{
	paddle := &world.paddle;
	ball := &world.ball;

	paddle.x = cast(f32)(GAME_WIDTH / 2);
	paddle.y = cast(f32)(GAME_HEIGHT / 10 * 9);
	paddle.width = 40.0;
	paddle.height = 10.0;

	ball.width = 10.0;
	ball.height = 10.0;

	world.num_bricks = 12;

	world.lives = 3;
	world.level = 1;

	foreach i : 0..<world.num_bricks
	{
		brick := &world.bricks[i];

		brick.width = 40.0;
		brick.height = 10.0;
		brick.x = 35.0 + (brick.width  + 10.0) * cast(f32)(i%6);
		brick.y = 20.0 + (brick.height + 10.0) * cast(f32)(i/6);
	}
}

world_update :: proc(world: *World, game: *Game, delta: f32)
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

	switch ball.ball_state
	{
		case .stick_to_paddle:
		{
			ball.x = paddle.x;
			ball.y = paddle.y - (paddle.height + ball.height)*0.5;

			if IsKeyPressed(KEY_SPACE)
			{
				ball.hspeed = 2.0;
				ball.vspeed = -2.0;
				ball.ball_state = .move;
			}
		}

		case .move:
		{
			ball.x += ball.hspeed * delta;
			ball.y += ball.vspeed * delta;

			if ball.x + ball.width*0.5 >= cast(f32)GAME_WIDTH
			{
				ball.hspeed = -fabsf(ball.hspeed);
			}
			if ball.x - ball.width*0.5 < 0.0
			{
				ball.hspeed = fabsf(ball.hspeed);
			}

			if ball.y - ball.height*0.5 < 0.0
			{
				ball.vspeed = fabsf(ball.vspeed);
			}

			if ball.y + ball.height*0.5 >= cast(f32)GAME_HEIGHT
			{
				world.lives -= 1;
				ball.ball_state = .stick_to_paddle;
			}

			if entities_collide(ball, paddle)
			{
				ball.vspeed = -fabsf(ball.vspeed);
			}

			foreach i : 0..<world.num_bricks
			{
				brick := &world.bricks[i];

				if brick.dead
				{
					continue;
				}

				if entities_collide(ball, brick)
				{
					ball.vspeed = -ball.vspeed;
					ball.vspeed *= 1.1;
					ball.hspeed *= 1.1;
					brick.dead = true;
				}
			}
		}
	}

	// lose condition
	if world.lives == 0
	{
		game.state = .title;
	}

	// TODO: win condition
}

entity_draw :: proc(entity: *Entity, color: u32)
{
	DrawRectangle(cast(i32)(entity.x - entity.width*0.5),
				  cast(i32)(entity.y - entity.height*0.5),
				  cast(i32)entity.width,
				  cast(i32)entity.height,
				  GetColor(color));
}

world_draw_ui :: proc(world: *World)
{
	draw_text_centered(TextFormat("LEVEL: %lld"c, world.level), 100, 0, 10, 0xffffffff);

	draw_text_centered(TextFormat("LIVES: %lld"c, world.lives), 200, 0, 10, 0xffffffff);
}

world_draw :: proc(world: *World)
{
	entity_draw(&world.paddle, 0xffffffff);

	foreach i : 0..<world.num_bricks
	{
		brick := &world.bricks[i];

		if brick.dead
		{
			continue;
		}

		entity_draw(brick, 0xffffffff);
	}

	entity_draw(&world.ball, 0xffffffff);

	world_draw_ui(world);
}

entities_collide :: proc(entity1: *Entity, entity2: *Entity) -> bool
{
	return rect_vs_rect(entity1.x - entity1.width*0.5,
						entity1.y - entity1.height*0.5,
						entity1.width,
						entity1.height,
						entity2.x - entity2.width*0.5,
						entity2.y - entity2.height*0.5,
						entity2.width,
						entity2.height);
}

rect_vs_rect :: proc(x1: f32, y1: f32, width1: f32, height1: f32,
					 x2: f32, y2: f32, width2: f32, height2: f32) -> bool
{
	return (x1 < x2+width2
			&& x1+width1 > x2
			&& y1 < y2+height2
			&& y1+height1 > y2);
}
