#link_library "gamelib.lib";

window_create :: proc(title: *u8, width: i32, height: i32) #foreign;

window_close :: proc() #foreign;

window_should_close :: proc() -> bool #foreign;

handle_events :: proc() #foreign;

swap_buffers :: proc() #foreign;
