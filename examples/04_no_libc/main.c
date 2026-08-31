#use_vendor_lld;
#link_library "kernel32_imp.lib";
#link_library "msvcrt_imp.lib";

#import "gamelib/gamelib.c"

ExitProcess :: proc(code: u32) #foreign;

mainCRTStartup :: proc() -> int #foreign
{
	window_create("test"c, 640, 480);

	reimu_idle := load_texture("reimu_idle.png"c);

	while !window_should_close()
	{
		handle_events();

		clear_color(color_from_u32(0x000000ff));

		begin_drawing();

		draw_texture(reimu_idle, 0, 0);

		end_drawing();

		swap_buffers();
	}
	
	window_close();

	ExitProcess(0);
}
