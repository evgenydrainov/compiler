vec3 :: proc(x: f32, y: f32, z: f32) -> vec3
{
	result: vec3 = ---;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

vec3_length :: proc(v: vec3) -> f32
{
	return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

vec3_abs :: proc(v: vec3) -> vec3
{
	result: vec3 = ---;
	result.x = fabsf(v.x);
	result.y = fabsf(v.y);
	result.z = fabsf(v.z);
	return result;
}



__add_vec3_vec3 :: proc(a: vec3, b: vec3) -> vec3
{
	result: vec3 = ---;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

__sub_vec3_vec3 :: proc(a: vec3, b: vec3) -> vec3
{
	result: vec3 = ---;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

__mul_vec3_vec3 :: proc(a: vec3, b: vec3) -> vec3
{
	result: vec3 = ---;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}

__div_vec3_vec3 :: proc(a: vec3, b: vec3) -> vec3
{
	result: vec3 = ---;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	return result;
}

__mul_vec3_f32 :: proc(a: vec3, b: f32) -> vec3
{
	result: vec3 = ---;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

__div_vec3_f32 :: proc(a: vec3, b: f32) -> vec3
{
	result: vec3 = ---;
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	return result;
}

__mul_f32_vec3 :: proc(a: f32, b: vec3) -> vec3
{
	result: vec3 = ---;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
	return result;
}

__div_f32_vec3 :: proc(a: f32, b: vec3) -> vec3
{
	result: vec3 = ---;
	result.x = a / b.x;
	result.y = a / b.y;
	result.z = a / b.z;
	return result;
}
