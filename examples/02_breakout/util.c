// Enter, but not Alt+Enter (that one toggles borderless windowed)
is_confirm_pressed :: proc() -> bool
{
	return (IsKeyPressed(KEY_ENTER)
			&& !IsKeyDown(KEY_LEFT_ALT)
			&& !IsKeyDown(KEY_RIGHT_ALT));
}

// scale the alpha of a 0xRRGGBBAA color by amount (0..1)
color_fade :: proc(color: u32, amount: f32) -> u32
{
	alpha := cast(u32)(cast(f32)(color & 0xff) * amount);
	return (color & 0xffffff00) | alpha;
}

draw_text_centered :: proc(text: *u8,
						   x: int, y: int,
						   font_size: i32, color: u32)
{
	text_width := MeasureText(text, font_size);

	DrawText(text,
			 cast(i32)(x - text_width/2),
			 cast(i32)y,
			 font_size,
			 GetColor(color));
}
