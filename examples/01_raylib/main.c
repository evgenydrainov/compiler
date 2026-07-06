asm
{
	section .data

	__tilemap_data:
		dq 33,34,33,34,33,34,33,34,33,34,33,34,33,34,33,34
		dq 49,50,49,50,49,50,49,50,49,50,49,50,49,50,49,50
		dq 33,34,33,34,1,1,1,2,1,34,33,34,33,34,33,34
		dq 49,50,49,1,1,1,1,11,1,50,49,50,49,50,49,50
		dq 33,34,33,1,1,1,1,27,1,34,33,34,33,34,33,34
		dq 49,50,49,1,1,1,49,2,49,50,49,50,49,50,49,50
		dq 33,34,33,1,1,2,2,2,2,2,33,34,2,2,33,34
		dq 49,50,49,1,1,1,1,1,1,1,1,1,1,50,49,50
		dq 33,34,33,1,1,1,1,1,1,1,1,1,1,34,33,34
		dq 49,50,49,50,49,1,1,1,1,1,1,2,2,50,49,50
		dq 33,34,2,34,2,1,2,2,2,1,2,34,33,34,33,34
		dq 49,50,49,50,49,1,1,1,1,1,1,50,49,50,49,50
		dq 33,34,33,34,33,34,33,34,33,34,33,34,33,34,33,34
		dq 49,50,49,50,49,50,49,50,49,50,49,50,49,50,49,50
		dq 33,34,33,34,33,34,33,34,33,34,33,34,33,34,33,34
		dq 49,50,49,50,49,50,49,50,49,50,49,50,49,50,49,50

	section .text
}

#include "../../modules/builtin.c"
#include "../../modules/raylib/raylib.c"

Player :: struct
{
	x: i64;
	y: i64;
	xspeed: i64;
	yspeed: i64;

	srcX: i64;
	srcY: i64;

	walkAnim: i64;
};

Tilemap :: struct
{
	data: *i64;
	width: i64;
	height: i64;
};

Game :: struct
{
	player: Player;

	tex_player: Texture;
	tex_tilemap: Texture;

	tilemap: Tilemap;
};

GameUpdate :: proc(game: *Game)
{
	KEY_RIGHT : i32 = 262;
	KEY_LEFT  : i32 = 263;
	KEY_DOWN  : i32 = 264;
	KEY_UP    : i32 = 265;
	KEY_Z     : i32 = 90;

	dirX := 0;
	if IsKeyDown(KEY_LEFT)
	{
		dirX -= 1;
	}
	if IsKeyDown(KEY_RIGHT)
	{
		dirX += 1;
	}

	game.player.xspeed = 0x2_0000*dirX;

	if IsKeyPressed(KEY_Z)
	{
		game.player.yspeed = -0x4_0000;
	}

	{
		gravity := 0x0_2000;

		game.player.yspeed += gravity;
	}

	{
		checkX := game.player.x + game.player.xspeed;
		if CheckTilemapCollision(&game.tilemap, checkX>>16, game.player.y>>16)
		{
			game.player.xspeed = 0;
		}
		else
		{
			game.player.x = checkX;
		}
	}

	{
		checkY := game.player.y + game.player.yspeed;
		if CheckTilemapCollision(&game.tilemap, game.player.x>>16, checkY>>16)
		{
			game.player.yspeed = 0;
		}
		else
		{
			game.player.y = checkY;
		}
	}

	if game.player.y > ((240-8)<<16)
	{
		game.player.y = ((240-8)<<16);
		game.player.yspeed = 0;
	}

	if game.player.xspeed > 0
	{
		game.player.srcY = 16;
	}
	if game.player.xspeed < 0
	{
		game.player.srcY = 0;
	}

	if game.player.xspeed == 0
	{
		game.player.srcX = 0;
	}
	else
	{
		game.player.walkAnim += 1;
		game.player.walkAnim %= 16;

		game.player.srcX = (game.player.walkAnim / 8) * 16;
	}
}

GameDraw :: proc(game: *Game)
{
	camera: Camera2D;
	camera.zoom = int64_to_float32(4);

	BeginMode2D(&camera);

	for y := 0; y < game.tilemap.height; y += 1
	{
		for x := 0; x < game.tilemap.width; x += 1
		{
			tile := GetTile(&game.tilemap, x, y);
			draw_texture(&game.tex_tilemap,
						 (tile%16)*16, (tile/16)*16, 16, 16,
						 x*16, y*16);
		}
	}

	draw_texture(&game.tex_player,
				 game.player.srcX, game.player.srcY, 16, 16,
				 (game.player.x>>16)-8, (game.player.y>>16)-8);

	EndMode2D();
}

draw_texture :: proc(texture: *Texture,
					 srcX: i64, srcY: i64, srcWidth: i64, srcHeight: i64,
					 posX: i64, posY: i64)
{
	source: Rectangle;
	source.x      = int64_to_float32(srcX);
	source.y      = int64_to_float32(srcY);
	source.width  = int64_to_float32(srcWidth);
	source.height = int64_to_float32(srcHeight);

	position: Vector2;
	position.x = int64_to_float32(posX);
	position.y = int64_to_float32(posY);

	DrawTextureRec(texture, &source, position, 0xffffffff);
}

main :: proc() -> i64
{
	InitWindow(1280, 960, "doukutsu"c);
	SetTargetFPS(60);

	game: Game;
	game.player.x = 160<<16;
	game.player.y = 130<<16;

	LoadTexture(&game.tex_player, "player.png"c);
	LoadTexture(&game.tex_tilemap, "tilemap.png"c);

	game.tilemap.data = get_tilemap_data();
	game.tilemap.width = 16;
	game.tilemap.height = 16;

	while !WindowShouldClose()
	{
		GameUpdate(&game);

		BeginDrawing();

		ClearBackground(0xff000000);

		GameDraw(&game);

		EndDrawing();
	}

	return 0;
}

get_tilemap_data :: proc() -> *i64
{
	asm
	{
		lea rax, [rel __tilemap_data]
	}
}

GetTile :: proc(tilemap: *Tilemap,
				tileX: i64, tileY: i64) -> i64
{
	tile := 0;

	if tileX >= 0
		&& tileX < tilemap.width
		&& tileY >= 0
		&& tileY < tilemap.height
	{
		index := tileX + tileY * tilemap.width;
		tile = tilemap.data[index] - 1;
	}

	return tile;
}

CheckTilemapCollision :: proc(tilemap: *Tilemap,
							  pixelX: i64, pixelY: i64) -> bool
{
	tileX := pixelX / 16;
	tileY := pixelY / 16;

	tile := GetTile(tilemap, tileX, tileY);

	return tile != 0;
}
