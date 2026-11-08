vec4_length :: proc(v: vec4) -> f32
{
	return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

__add_vec4_vec4 :: proc(a: vec4, b: vec4) -> vec4
{
	result: vec4;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
	return result;
}

__sub_vec4_vec4 :: proc(a: vec4, b: vec4) -> vec4
{
	result: vec4;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = a.w - b.w;
	return result;
}

__mul_vec4_vec4 :: proc(a: vec4, b: vec4) -> vec4
{
	result: vec4;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	result.w = a.w * b.w;
	return result;
}

__div_vec4_vec4 :: proc(a: vec4, b: vec4) -> vec4
{
	result: vec4;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	result.w = a.w / b.w;
	return result;
}

__mul_vec4_f32 :: proc(a: vec4, b: f32) -> vec4
{
	result: vec4;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	result.w = a.w * b;
	return result;
}

__div_vec4_f32 :: proc(a: vec4, b: f32) -> vec4
{
	result: vec4;
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	result.w = a.w / b;
	return result;
}

__mul_f32_vec4 :: proc(a: f32, b: vec4) -> vec4
{
	result: vec4;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
	result.w = a * b.w;
	return result;
}

__div_f32_vec4 :: proc(a: f32, b: vec4) -> vec4
{
	result: vec4;
	result.x = a / b.x;
	result.y = a / b.y;
	result.z = a / b.z;
	result.w = a / b.w;
	return result;
}
