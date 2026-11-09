// Enter, but not Alt+Enter (that one toggles borderless windowed)
is_confirm_pressed :: proc() -> bool
{
	return (IsKeyPressed(KEY_ENTER)
			&& !IsKeyDown(KEY_LEFT_ALT)
			&& !IsKeyDown(KEY_RIGHT_ALT));
}

// scale the alpha of a color by amount (0..1)
color_fade :: proc(color: vec4, amount: f32) -> vec4
{
	color.a *= amount;
	return color;
}

draw_text_centered :: proc(text: *u8,
						   x: int, y: int,
						   font_size: i32, color: vec4)
{
	text_width := MeasureText(text, font_size);
	DrawText(text,
			 cast(i32)(x - text_width/2),
			 cast(i32)y,
			 font_size,
			 color_to_raylib(color));
}

draw_text_centered2 :: proc(text: *u8,
							x: int, y: int,
							font_size: i32)
{
	draw_text_centered(text, x, y, font_size, vec4(1, 1, 1, 1));
}

color_from_u32 :: proc(color: u32) -> vec4
{
	result: vec4;
	result.r = cast(f32)((color >> 24) & 0xff) / 255.0;
	result.g = cast(f32)((color >> 16) & 0xff) / 255.0;
	result.b = cast(f32)((color >> 8)  & 0xff) / 255.0;
	result.a = cast(f32)((color >> 0)  & 0xff) / 255.0;
	return result;
}

color_to_raylib :: proc(color: vec4) -> Color
{
	result: Color;
	result.r = cast(u8)(color.r * 255.0);
	result.g = cast(u8)(color.g * 255.0);
	result.b = cast(u8)(color.b * 255.0);
	result.a = cast(u8)(color.a * 255.0);
	return result;
}
