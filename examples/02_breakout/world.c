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

	color: u32;
	points: int;

	// bricks pop in one by one at the start of a level
	appear_time: f32;

	dead: bool;
};

WorldState :: enum
{
	playing;
	ball_lost;
	level_clear;
	paused;
	game_over;
};

// all times are in frames

// how long the "LEVEL N" banner stays up, and you can't launch the ball
LEVEL_BANNER_TIME :: 90;

// pause between clearing a level and the next one starting
LEVEL_CLEAR_TIME :: 90;

BRICK_APPEAR_INTERVAL :: 2;
BRICK_APPEAR_DURATION :: 8;

// pause after the ball falls off the screen
BALL_LOST_TIME :: 60;

// game over screen: the overlay fades in, then the text,
// then you can press enter
GAME_OVER_OVERLAY_FADE_END :: 40;
GAME_OVER_TEXT_FADE_START  :: 30;
GAME_OVER_TEXT_FADE_END    :: 60;
GAME_OVER_INPUT_TIME       :: 90;

World :: struct
{
	state: WorldState;
	level_banner_timer: f32;
	level_clear_timer: f32;
	ball_lost_timer: f32;
	game_over_time: f32;

	// time since the level started
	level_time: f32;

	paddle: Entity;
	ball: Entity;

	bricks: [..]Entity;

	lives: int;
	level: int;
	score: int;

	new_high_score: bool;
};

world_deinit :: proc(world: *World)
{
	array_free(&world.bricks);
	
	zero_world: World;
	*world = zero_world;
}

world_init :: proc(world: *World)
{
	world_deinit(world);

	paddle := &world.paddle;
	ball := &world.ball;

	paddle.x = GAME_WIDTH/2;
	paddle.y = GAME_HEIGHT/10*9;
	paddle.width = 40;
	paddle.height = 10;

	ball.width = 10;
	ball.height = 10;

	world.lives = 3;
	world.level = 1;

	world_spawn_bricks(world);
}

BALL_LAUNCH_SPEED :: 4;
BALL_MAX_SPEED    :: 6;
BALL_SPEEDUP      :: 1.05;

BRICK_COLUMNS  :: 6;
BRICK_MAX_ROWS :: 6;

// one more row of bricks per level
world_spawn_bricks :: proc(world: *World)
{
	world.bricks.count = 0;

	rows := world.level + 1;
	if rows > BRICK_MAX_ROWS
	{
		rows = BRICK_MAX_ROWS;
	}

	for i in 0..<BRICK_COLUMNS*rows
	{
		brick: Entity;
		brick.width = 40;
		brick.height = 10;
		brick.x = 35 + (brick.width  + 10) * cast(f32)(i%BRICK_COLUMNS);
		brick.y = 20 + (brick.height + 10) * cast(f32)(i/BRICK_COLUMNS);
		brick.color = brick_row_color(i/BRICK_COLUMNS);

		// higher rows are worth more
		brick.points = (rows - i/BRICK_COLUMNS) * 10;

		brick.appear_time = cast(f32)(i * BRICK_APPEAR_INTERVAL);

		array_add(&world.bricks, brick);
	}

	world.level_banner_timer = LEVEL_BANNER_TIME;
	world.level_time = 0;
}

brick_row_color :: proc(row: int) -> u32
{
	switch row
	{
		case 0:  return 0xe04040ff; // red
		case 1:  return 0xe08a30ff; // orange
		case 2:  return 0xe0d040ff; // yellow
		case 3:  return 0x50c050ff; // green
		case 4:  return 0x4080e0ff; // blue
		default: return 0xa050d0ff; // purple
	}
}

is_pause_pressed :: proc() -> bool
{
	return IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE);
}

// the further from the paddle's center the ball hits, the steeper the angle
ball_bounce_off_paddle :: proc(ball: *Entity, paddle: *Entity)
{
	speed := sqrtf(ball.hspeed*ball.hspeed + ball.vspeed*ball.vspeed);

	offset := (ball.x - paddle.x) / (paddle.width*0.5);
	offset = fmaxf(offset, -1);
	offset = fminf(offset,  1);

	// keep at least some vertical speed so the ball never goes flat
	ball.hspeed = speed * offset * 0.8;
	ball.vspeed = -sqrtf(speed*speed - ball.hspeed*ball.hspeed);
}

// bounce along the axis with the smallest penetration
ball_bounce_off_brick :: proc(ball: *Entity, brick: *Entity)
{
	dx := ball.x - brick.x;
	dy := ball.y - brick.y;

	overlap_x := (ball.width  + brick.width) *0.5 - fabsf(dx);
	overlap_y := (ball.height + brick.height)*0.5 - fabsf(dy);

	if overlap_x < overlap_y
	{
		if dx < 0
		{
			ball.hspeed = -fabsf(ball.hspeed);
		}
		else
		{
			ball.hspeed =  fabsf(ball.hspeed);
		}
	}
	else
	{
		if dy < 0
		{
			ball.vspeed = -fabsf(ball.vspeed);
		}
		else
		{
			ball.vspeed =  fabsf(ball.vspeed);
		}
	}

	// speed up a little with every brick
	speed := sqrtf(ball.hspeed*ball.hspeed + ball.vspeed*ball.vspeed);
	if speed * BALL_SPEEDUP <= BALL_MAX_SPEED
	{
		ball.hspeed *= BALL_SPEEDUP;
		ball.vspeed *= BALL_SPEEDUP;
	}
}

world_update :: proc(world: *World, game: *Game, delta: f32)
{
	switch world.state
	{
	case .playing:
		world_update_playing(world, delta);

	case .ball_lost:
		world_update_ball_lost(world, delta);

		if world.state == .game_over && world.score > game.high_score
		{
			game.high_score = world.score;
			world.new_high_score = true;
		}

	case .level_clear:
		world_update_level_clear(world, delta);

	case .paused:
		if is_pause_pressed()
		{
			world.state = .playing;
		}

	case .game_over:
		world.game_over_time += delta;

		if world.game_over_time >= GAME_OVER_INPUT_TIME && is_confirm_pressed()
		{
			game.state = .title;
		}
	}
}

world_update_playing :: proc(world: *World, delta: f32)
{
	paddle := &world.paddle;
	ball := &world.ball;

	if is_pause_pressed()
	{
		world.state = .paused;
		return;
	}

	world.level_banner_timer = fmaxf(world.level_banner_timer - delta, 0);
	world.level_time += delta;

	paddle_update(paddle, delta);

	switch ball.ball_state
	{
	case .stick_to_paddle:
		ball.x = paddle.x;
		ball.y = paddle.y - (paddle.height + ball.height)*0.5;

		// wait for the bricks to appear
		if world.level_banner_timer == 0 && IsKeyPressed(KEY_SPACE)
		{
			// launch straight up, aim with the paddle
			ball.hspeed = 0;
			ball.vspeed = -BALL_LAUNCH_SPEED;
			ball.ball_state = .move;
		}

	case .move:
		ball.x += ball.hspeed * delta;
		ball.y += ball.vspeed * delta;

		if ball.x + ball.width*0.5 >= GAME_WIDTH
		{
			ball.hspeed = -fabsf(ball.hspeed);
		}
		if ball.x - ball.width*0.5 < 0
		{
			ball.hspeed = fabsf(ball.hspeed);
		}

		if ball.y - ball.height*0.5 < 0
		{
			ball.vspeed = fabsf(ball.vspeed);
		}

		// let it fall all the way off the screen
		if ball.y - ball.height*0.5 >= GAME_HEIGHT
		{
			world.lives -= 1;
			world.state = .ball_lost;
			world.ball_lost_timer = BALL_LOST_TIME;
			return;
		}

		// only when moving down, so it can't get stuck inside the paddle
		if ball.vspeed > 0 && entities_collide(ball, paddle)
		{
			ball_bounce_off_paddle(ball, paddle);
		}

		for *brick in world.bricks
		{
			if entities_collide(ball, brick)
			{
				ball_bounce_off_brick(ball, brick);
				brick.dead = true;
				world.score += brick.points;

				// only bounce off one brick per frame
				break;
			}
		}
	}

	// cleanup
	{
		i := 0;
		while i < world.bricks.count
		{
			if world.bricks[i].dead
			{
				array_unordered_remove(&world.bricks, i);
			}
			else
			{
				i += 1;
			}
		}
	}

	// win condition: short pause, then the next level
	if world.bricks.count == 0
	{
		world.state = .level_clear;
		world.level_clear_timer = LEVEL_CLEAR_TIME;
		ball.ball_state = .stick_to_paddle;
	}
}

world_update_ball_lost :: proc(world: *World, delta: f32)
{
	paddle_update(&world.paddle, delta);

	world.ball_lost_timer -= delta;
	if world.ball_lost_timer <= 0
	{
		if world.lives == 0
		{
			world.state = .game_over;
			world.game_over_time = 0;
		}
		else
		{
			world.ball.ball_state = .stick_to_paddle;
			world.state = .playing;
		}
	}
}

world_update_level_clear :: proc(world: *World, delta: f32)
{
	// you can still move the paddle around
	paddle_update(&world.paddle, delta);

	world.level_clear_timer -= delta;
	if world.level_clear_timer <= 0
	{
		world.level += 1;
		world_spawn_bricks(world);
		world.state = .playing;
	}
}

PADDLE_SPEED :: 5;

paddle_update :: proc(paddle: *Entity, delta: f32)
{
	if IsKeyDown(KEY_RIGHT)
	{
		paddle.x += PADDLE_SPEED*delta;
	}
	if IsKeyDown(KEY_LEFT)
	{
		paddle.x -= PADDLE_SPEED*delta;
	}

	paddle.x = fmaxf(paddle.x, paddle.width*0.5);
	paddle.x = fminf(paddle.x, GAME_WIDTH - paddle.width*0.5);
}

entity_draw :: proc(entity: *Entity, color: u32)
{
	entity_draw_scaled(entity, color, 1);
}

entity_draw_scaled :: proc(entity: *Entity, color: u32, scale: f32)
{
	width  := entity.width  * scale;
	height := entity.height * scale;

	DrawRectangle(cast(i32)(entity.x - width*0.5),
				  cast(i32)(entity.y - height*0.5),
				  cast(i32)width,
				  cast(i32)height,
				  GetColor(color));
}

world_draw_ui :: proc(world: *World)
{
	// small margin from the top edge; the top row of bricks starts at y=15
	draw_text_centered(TextFormat("LEVEL: %lld"c, world.level), 60,  3, 10, 0xffffffff);
	draw_text_centered(TextFormat("SCORE: %lld"c, world.score), 160, 3, 10, 0xffffffff);
	draw_text_centered(TextFormat("LIVES: %lld"c, world.lives), 260, 3, 10, 0xffffffff);

	x := GAME_WIDTH/2;
	y := GAME_HEIGHT/2;

	switch world.state
	{
	case .playing:
		if world.level_banner_timer > 0
		{
			draw_text_centered(TextFormat("LEVEL %lld"c, world.level), x, y, 20, 0xffffffff);
		}
		else if world.ball.ball_state == .stick_to_paddle
		{
			draw_text_centered("PRESS SPACE TO LAUNCH"c, x, y + 20, 10, 0xffffffff);
		}

	case .level_clear:
		draw_text_centered(TextFormat("LEVEL %lld CLEAR!"c, world.level), x, y, 20, 0xe0d040ff);

	case .paused:
		draw_dim_overlay(1);
		draw_text_centered("PAUSED"c, x, y - 10, 20, 0xffffffff);
		draw_text_centered("PRESS P TO RESUME"c, x, y + 20, 10, 0xffffffff);

	case .game_over:
		t := world.game_over_time;

		draw_dim_overlay(fminf(t / GAME_OVER_OVERLAY_FADE_END, 1));

		text_alpha := (t - GAME_OVER_TEXT_FADE_START) / (GAME_OVER_TEXT_FADE_END - GAME_OVER_TEXT_FADE_START);
		text_alpha = fmaxf(text_alpha, 0);
		text_alpha = fminf(text_alpha, 1);

		white  := color_fade(0xffffffff, text_alpha);
		yellow := color_fade(0xe0d040ff, text_alpha);

		draw_text_centered("GAME OVER"c, x, y - 40, 20, white);
		draw_text_centered(TextFormat("YOU REACHED LEVEL %lld"c, world.level), x, y - 10, 10, white);
		draw_text_centered(TextFormat("SCORE: %lld"c, world.score), x, y + 5, 10, white);
		if world.new_high_score
		{
			draw_text_centered("NEW HIGH SCORE!"c, x, y + 20, 10, yellow);
		}

		if t >= GAME_OVER_INPUT_TIME
		{
			draw_text_centered("PRESS ENTER"c, x, y + 45, 10, white);
		}
	}
}

draw_dim_overlay :: proc(amount: f32)
{
	DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, GetColor(color_fade(0x000000b0, amount)));
}

world_draw :: proc(world: *World)
{
	entity_draw(&world.paddle, 0xffffffff);

	for *brick in world.bricks
	{
		// grow from nothing to full size
		t := (world.level_time - brick.appear_time) / BRICK_APPEAR_DURATION;
		if t > 0
		{
			entity_draw_scaled(brick, brick.color, fminf(t, 1));
		}
	}

	// game over: the ball is lost, it's just sitting at the bottom of the screen
	// level clear: the ball is gone until the next level starts
	if world.state != .game_over && world.state != .level_clear
	{
		entity_draw(&world.ball, 0xffffffff);
	}

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
