BallState :: enum
{
	stick_to_paddle;
	move;
};

Entity :: struct
{
	pos: vec2;
	vel: vec2;
	size: vec2;

	ball_state: BallState;

	color: vec4;
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

	paddle.pos = vec2(GAME_WIDTH/2, GAME_HEIGHT/10*9);
	paddle.size = vec2(40, 10);

	ball.size = vec2(10, 10);

	world.lives = 3;
	world.level = 1;

	world_spawn_bricks(world);
}

BALL_LAUNCH_SPEED :: 4;
BALL_MAX_SPEED    :: 5.5;
BALL_SPEEDUP      :: 1.05;

BRICK_COLUMNS  :: 6;
BRICK_MAX_ROWS :: 6;

// one more row of bricks per level
world_spawn_bricks :: proc(world: *World)
{
	world.bricks.count = 0;

	rows := min_i64(world.level + 1, BRICK_MAX_ROWS);

	for i in 0..<BRICK_COLUMNS*rows
	{
		brick: Entity;
		brick.size = vec2(40, 10);
		brick.pos = vec2(35 + (brick.size.x + 10) * cast(f32)(i%BRICK_COLUMNS),
						 20 + (brick.size.y + 10) * cast(f32)(i/BRICK_COLUMNS));
		brick.color = brick_row_color(i/BRICK_COLUMNS);

		// higher rows are worth more
		brick.points = (rows - i/BRICK_COLUMNS) * 10;

		brick.appear_time = cast(f32)(i * BRICK_APPEAR_INTERVAL);

		array_add(&world.bricks, brick);
	}

	world.level_banner_timer = LEVEL_BANNER_TIME;
	world.level_time = 0;
}

brick_row_color :: proc(row: int) -> vec4
{
	switch row
	{
		case 0:  return color_from_u32(0xe04040ff); // red
		case 1:  return color_from_u32(0xe08a30ff); // orange
		case 2:  return color_from_u32(0xe0d040ff); // yellow
		case 3:  return color_from_u32(0x50c050ff); // green
		case 4:  return color_from_u32(0x4080e0ff); // blue
		default: return color_from_u32(0xa050d0ff); // purple
	}
}

is_pause_pressed :: proc() -> bool
{
	return IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE);
}

// the further from the paddle's center the ball hits, the steeper the angle
ball_bounce_off_paddle :: proc(ball: *Entity, paddle: *Entity)
{
	speed := vec2_length(ball.vel);

	offset := clamp_f32((ball.pos.x - paddle.pos.x) / (paddle.size.x*0.5), -1, 1);

	// keep at least some vertical speed so the ball never goes flat
	ball.vel = vec2(speed * offset * 0.8,
					-sqrtf(speed*speed - ball.vel.x*ball.vel.x));
}

// bounce along the axis with the smallest penetration
ball_bounce_off_brick :: proc(ball: *Entity, brick: *Entity)
{
	diff := ball.pos - brick.pos;

	overlap := (ball.size + brick.size)*0.5 - vec2_abs(diff);

	if overlap.x < overlap.y
	{
		ball.vel.x = fabsf(ball.vel.x) * sign_f32(diff.x);
	}
	else
	{
		ball.vel.y = fabsf(ball.vel.y) * sign_f32(diff.y);
	}

	// speed up a little with every brick
	speed := vec2_length(ball.vel);
	if speed*BALL_SPEEDUP <= BALL_MAX_SPEED
	{
		ball.vel *= BALL_SPEEDUP;
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
		ball.pos = paddle.pos - vec2(0, (paddle.size.y + ball.size.y)*0.5);

		// wait for the bricks to appear
		if world.level_banner_timer == 0 && IsKeyPressed(KEY_SPACE)
		{
			// launch straight up, aim with the paddle
			ball.vel = vec2(0, -BALL_LAUNCH_SPEED);
			ball.ball_state = .move;
		}

	case .move:
		ball.pos += ball.vel*delta;

		if ball.pos.x + ball.size.x*0.5 >= GAME_WIDTH
		{
			ball.vel.x = -fabsf(ball.vel.x);
		}
		if ball.pos.x - ball.size.x*0.5 < 0
		{
			ball.vel.x = fabsf(ball.vel.x);
		}

		if ball.pos.y - ball.size.y*0.5 < 0
		{
			ball.vel.y = fabsf(ball.vel.y);
		}

		// let it fall all the way off the screen
		if ball.pos.y - ball.size.y*0.5 >= GAME_HEIGHT
		{
			world.lives -= 1;
			world.state = .ball_lost;
			world.ball_lost_timer = BALL_LOST_TIME;
			return;
		}

		// only when moving down, so it can't get stuck inside the paddle
		if ball.vel.y > 0 && entities_collide(ball, paddle)
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
	paddle.vel = vec2(0, 0);

	if IsKeyDown(KEY_RIGHT)
	{
		paddle.vel.x += PADDLE_SPEED;
	}
	if IsKeyDown(KEY_LEFT)
	{
		paddle.vel.x -= PADDLE_SPEED;
	}
	
	paddle.pos += paddle.vel*delta;

	paddle.pos.x = clamp_f32(paddle.pos.x, paddle.size.x*0.5, GAME_WIDTH - paddle.size.x*0.5);
}

entity_draw2 :: proc(entity: *Entity, color: vec4, scale: f32)
{
	size := entity.size*scale;
	DrawRectangle(cast(i32)(entity.pos.x - size.x*0.5),
				  cast(i32)(entity.pos.y - size.y*0.5),
				  cast(i32)size.x,
				  cast(i32)size.y,
				  color_to_raylib(color));
}

entity_draw :: proc(entity: *Entity, color: vec4)
{
	entity_draw2(entity, color, 1);
}

entity_draw3 :: proc(entity: *Entity)
{
	entity_draw2(entity, vec4(1, 1, 1, 1), 1);
}

world_draw_ui :: proc(world: *World)
{
	// small margin from the top edge; the top row of bricks starts at y=15
	draw_text_centered2(TextFormat("LEVEL: %lld"c, world.level), 60,  3, 10);
	draw_text_centered2(TextFormat("SCORE: %lld"c, world.score), 160, 3, 10);
	draw_text_centered2(TextFormat("LIVES: %lld"c, world.lives), 260, 3, 10);

	x := GAME_WIDTH/2;
	y := GAME_HEIGHT/2;

	switch world.state
	{
	case .playing:
		if world.level_banner_timer > 0
		{
			draw_text_centered2(TextFormat("LEVEL %lld"c, world.level), x, y, 20);
		}
		else if world.ball.ball_state == .stick_to_paddle
		{
			draw_text_centered2("PRESS SPACE TO LAUNCH"c, x, y + 20, 10);
		}

	case .level_clear:
		draw_text_centered(TextFormat("LEVEL %lld CLEAR!"c, world.level), x, y, 20, color_from_u32(0xe0d040ff));

	case .paused:
		draw_dim_overlay(1);
		draw_text_centered2("PAUSED"c, x, y - 10, 20);
		draw_text_centered2("PRESS P TO RESUME"c, x, y + 20, 10);

	case .game_over:
		t := world.game_over_time;

		draw_dim_overlay(fminf(t / GAME_OVER_OVERLAY_FADE_END, 1));

		text_alpha := clamp_f32((t - GAME_OVER_TEXT_FADE_START) / (GAME_OVER_TEXT_FADE_END - GAME_OVER_TEXT_FADE_START), 0, 1);

		white  := vec4(1, 1, 1, text_alpha);
		yellow := color_fade(color_from_u32(0xe0d040ff), text_alpha);

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
	DrawRectangle(0, 0,
				  GAME_WIDTH, GAME_HEIGHT,
				  color_to_raylib(vec4(0, 0, 0, 0.70*amount)));
}

world_draw :: proc(world: *World)
{
	entity_draw3(&world.paddle);

	for *brick in world.bricks
	{
		// grow from nothing to full size
		t := (world.level_time - brick.appear_time) / BRICK_APPEAR_DURATION;
		if t > 0
		{
			entity_draw2(brick, brick.color, fminf(t, 1));
		}
	}

	// game over: the ball is lost, it's just sitting at the bottom of the screen
	// level clear: the ball is gone until the next level starts
	if world.state != .game_over && world.state != .level_clear
	{
		entity_draw3(&world.ball);
	}

	world_draw_ui(world);
}

entities_collide :: proc(e1: *Entity, e2: *Entity) -> bool
{
	return rect_vs_rect(e1.pos - e1.size*0.5, e1.size,
						e2.pos - e2.size*0.5, e2.size);
}

rect_vs_rect :: proc(pos1: vec2, size1: vec2,
					 pos2: vec2, size2: vec2) -> bool
{
	return (pos1.x < pos2.x+size2.x
			&& pos1.x+size1.x > pos2.x
			&& pos1.y < pos2.y+size2.y
			&& pos1.y+size1.y > pos2.y);
}
