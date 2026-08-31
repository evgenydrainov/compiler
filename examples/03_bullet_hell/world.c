Entity :: struct
{
	x: f32;
	y: f32;

	vel_x: f32;
	vel_y: f32;

	acc_x: f32;
	acc_y: f32;

	width: f32;
	height: f32;

	co: *mco_coro;

	wait_timer: f32;
};

this :: macro cast(*Entity)co.user_data;

wait :: proc(co: *mco_coro, time: f32)
{
	this.wait_timer += time;
	while this.wait_timer >= 1
	{
		mco_yield(co);
		this.wait_timer -= 1;
	}
}

enemy_script :: proc(co: *mco_coro)
{
	while true
	{
		wait(co, 60);

		launch_towards_point(co.user_data,
							 cast(f32)(rand()%GAME_WIDTH),
							 cast(f32)(rand()%GAME_HEIGHT),
							 0.1);
	}
}

World :: struct
{
	player: Entity;

	boss: Entity;
};

world_init :: proc(world: *World)
{
	player := &world.player;
	boss := &world.boss;

	player.x = GAME_WIDTH/2;
	player.y = GAME_HEIGHT/4*3;
	player.width = 32;
	player.height = 32;

	boss.x = GAME_WIDTH/2;
	boss.y = GAME_HEIGHT/4;
	boss.width = 32;
	boss.height = 32;

	desc := mco_desc_init(enemy_script, 0);
	mco_create(&boss.co, &desc);
}

world_update :: proc(world: *World, delta: f32)
{
	player := &world.player;
	boss := &world.boss;

	player_update(player, delta);

	boss.co.user_data = boss;
	mco_resume(boss.co);

	physics_update(player, delta);
	physics_update(boss, delta);
}

player_update :: proc(player: *Entity, delta: f32)
{
	dir_x := 0.0;
	dir_y := 0.0;

	if IsKeyDown(KEY_UP)
	{
		dir_y -= 1;
	}
	if IsKeyDown(KEY_DOWN)
	{
		dir_y += 1;
	}
	if IsKeyDown(KEY_LEFT)
	{
		dir_x -= 1;
	}
	if IsKeyDown(KEY_RIGHT)
	{
		dir_x += 1;
	}

	if dir_x != 0 && dir_y != 0
	{
		dir_x *= 0.70710678118654752440084436210485;
		dir_y *= 0.70710678118654752440084436210485;
	}

	move_speed := 4.5;

	player.vel_x = dir_x*move_speed;
	player.vel_y = dir_y*move_speed;
}

physics_update :: proc(entity: *Entity, delta: f32)
{
	entity.x += entity.vel_x*delta;
	entity.y += entity.vel_y*delta;

	entity.vel_x += entity.acc_x*delta;
	entity.vel_y += entity.acc_y*delta;
}

world_draw :: proc(world: *World)
{
	player := &world.player;
	boss := &world.boss;

	entity_draw(player, 0xffffffff);
	entity_draw(boss, 0xffffffff);
}

entity_draw :: proc(entity: *Entity, color: u32)
{
	DrawRectangle(cast(i32)(entity.x - entity.width*0.5),
				  cast(i32)(entity.y - entity.height*0.5),
				  cast(i32)entity.width,
				  cast(i32)entity.height,
				  GetColor(color));
}
