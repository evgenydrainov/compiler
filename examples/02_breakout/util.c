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
