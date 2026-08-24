point_distance :: proc(x1: f32, y1: f32, x2: f32, y2: f32) -> f32
{
	dx := x2 - x1;
	dy := y2 - y1;
	return sqrtf(dx*dx + dy*dy);
}

normalize0 :: proc(px: *f32, py: *f32)
{
	if *px!=0.0 || *py!=0.0
	{
		length := sqrtf((*px)*(*px) + (*py)*(*py));
		*px /= length;
		*py /= length;
	}
}

launch_towards_point :: proc(entity: *Entity, target_x: f32, target_y: f32, acc: f32)
{
	distance := point_distance(entity.x, entity.y, target_x, target_y);
	speed := sqrtf(2.0*distance*acc);

	dir_x := target_x - entity.x;
	dir_y := target_y - entity.y;
	normalize0(&dir_x, &dir_y);

	entity.vel_x = speed*dir_x;
	entity.vel_y = speed*dir_y;

	entity.acc_x = -acc*dir_x;
	entity.acc_y = -acc*dir_y;
}
