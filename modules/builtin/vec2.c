vec2 :: proc(x: f32, y: f32) -> vec2
{
	result: vec2 = ---;
	result.x = x;
	result.y = y;
	return result;
}

vec2_length :: proc(v: vec2) -> f32
{
	return sqrtf(v.x*v.x + v.y*v.y);
}

vec2_abs :: proc(v: vec2) -> vec2
{
	result: vec2 = ---;
	result.x = fabsf(v.x);
	result.y = fabsf(v.y);
	return result;
}



__add_vec2_vec2 :: proc(a: vec2, b: vec2) -> vec2
{
	result: vec2 = ---;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

__sub_vec2_vec2 :: proc(a: vec2, b: vec2) -> vec2
{
	result: vec2 = ---;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

__mul_vec2_vec2 :: proc(a: vec2, b: vec2) -> vec2
{
	result: vec2 = ---;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return result;
}

__div_vec2_vec2 :: proc(a: vec2, b: vec2) -> vec2
{
	result: vec2 = ---;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	return result;
}

__mul_vec2_f32 :: proc(a: vec2, b: f32) -> vec2
{
	result: vec2 = ---;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

__div_vec2_f32 :: proc(a: vec2, b: f32) -> vec2
{
	result: vec2 = ---;
	result.x = a.x / b;
	result.y = a.y / b;
	return result;
}

__mul_f32_vec2 :: proc(a: f32, b: vec2) -> vec2
{
	result: vec2 = ---;
	result.x = a * b.x;
	result.y = a * b.y;
	return result;
}

__div_f32_vec2 :: proc(a: f32, b: vec2) -> vec2
{
	result: vec2 = ---;
	result.x = a / b.x;
	result.y = a / b.y;
	return result;
}
