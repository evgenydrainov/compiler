main :: proc() -> i64
{
	KEY_RIGHT := 262;
	KEY_LEFT  := 263;
	KEY_DOWN  := 264;
	KEY_UP    := 265;
	KEY_Z     := 90;

	InitWindow(320, 240, "doukutsu"c);
	SetTargetFPS(60);

	player: Player;
	player.x = 160<<16;
	player.y = 120<<16;

	texture: Texture;
	LoadTexture(&texture, "plr.png"c);

	while WindowShouldClose() == false
	{
		BeginDrawing();

		ClearBackground(0);

		dirX := 0;
		if IsKeyDown(KEY_LEFT)
		{
			dirX = dirX - 1;
		}
		if IsKeyDown(KEY_RIGHT)
		{
			dirX = dirX + 1;
		}

		player.xspeed = 0x2_0000*dirX;

		if IsKeyPressed(KEY_Z)
		{
			player.yspeed = -0x4_0000;
		}

		{
			gravity := 0x0_2000;

			player.yspeed = player.yspeed + gravity;
		}

		player.x = player.x + player.xspeed;
		player.y = player.y + player.yspeed;

		if player.y > ((240-8)<<16)
		{
			player.y = ((240-8)<<16);
			player.yspeed = 0;
		}

		DrawTexture(&texture,
					(player.x>>16)-8,
					(player.y>>16)-8,
					0xffffffff);

		EndDrawing();
	}

	return 0;
}

Player :: struct
{
	x: i64;
	y: i64;
	xspeed: i64;
	yspeed: i64;
};

InitWindow :: proc(width: i64, height: i64, title: *i8) #foreign;
WindowShouldClose :: proc() -> bool #foreign;
CloseWindow :: proc() -> bool #foreign;
BeginDrawing :: proc() -> bool #foreign;
EndDrawing :: proc() -> bool #foreign;
SetTargetFPS :: proc(fps: i64) #foreign;
DrawRectangle :: proc(posX: i64, posY: i64, width: i64, height: i64, color: i64) #foreign;
DrawPixel :: proc(posX: i64, posY: i64, color: i64) #foreign;
IsKeyDown :: proc(key: i64) -> bool #foreign;
ClearBackground :: proc(color: i64) #foreign;
LoadTexture :: proc(texture: *Texture, fileName: *i8) #foreign;
DrawTexture :: proc(texture: *Texture, posX: i64, posY: i64, tint: i64) #foreign;
IsKeyPressed :: proc(key: i64) -> bool #foreign;

Texture :: struct
{
	id: i32;
	width: i32;
	height: i32;
	mipmaps: i32;
	format: i32;
};

Camera2D :: struct
{
	offsetX: i32;
	offsetY: i32;
	targetX: i32;
	targetY: i32;
	rotation: i32;
	zoom: i32;
};
