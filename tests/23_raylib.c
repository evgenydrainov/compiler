main :: proc() -> i64
{
	RIGHT: i64 = 262;
	LEFT: i64 = 263;
	DOWN: i64 = 264;
	UP: i64 = 265;

	windowTitle: i64 = 0;

	InitWindow(800, 600, &windowTitle);
	SetTargetFPS(60);

	player: Player;
	player.x = 400;
	player.y = 300;

	while WindowShouldClose() == false
	{
		BeginDrawing();

		ClearBackground(0);

		if IsKeyDown(UP)
		{
			player.y = player.y - 5;
		}
		if IsKeyDown(DOWN)
		{
			player.y = player.y + 5;
		}
		if IsKeyDown(LEFT)
		{
			player.x = player.x - 5;
		}
		if IsKeyDown(RIGHT)
		{
			player.x = player.x + 5;
		}

		draw_rect(player.x, player.y, 20, 20);

		EndDrawing();
	}

	return 0;
}

Player :: struct
{
	x: i64;
	y: i64;
}

draw_rect :: proc(x: i64, y: i64, width: i64, height: i64)
{
	for xx: i64 = 0; xx < width; xx = xx + 1;
	{
		for yy: i64 = 0; yy < height; yy = yy + 1;
		{
			DrawPixel(x + xx, y + yy, 0xffffffff);
		}
	}
}

InitWindow :: proc(width: i64, height: i64, title: *i64) #foreign;
WindowShouldClose :: proc() -> bool #foreign;
CloseWindow :: proc() -> bool #foreign;
BeginDrawing :: proc() -> bool #foreign;
EndDrawing :: proc() -> bool #foreign;
SetTargetFPS :: proc(fps: i64) #foreign;
DrawRectangle :: proc(posX: i64, posY: i64, width: i64, height: i64, color: i64) #foreign;
DrawPixel :: proc(posX: i64, posY: i64, color: i64) #foreign;
IsKeyDown :: proc(key: i64) -> bool #foreign;
ClearBackground :: proc(color: i64) #foreign;
