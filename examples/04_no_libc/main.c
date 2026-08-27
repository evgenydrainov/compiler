#import "gamelib/gamelib.c"

ExitProcess :: proc(code: u32) #foreign;

mainCRTStartup :: proc() -> int #foreign
{
	window_create("test"c, 640, 480);

	while !window_should_close()
	{
		handle_events();

		swap_buffers();
	}
	
	window_close();

	ExitProcess(0);
}
