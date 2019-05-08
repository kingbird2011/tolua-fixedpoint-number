#include "math-sll.h"

static void create_meta(lua_State *L);

void push_Vector3(lua_State *L, sll x, sll y, sll z)
{
	Vector3* p = lua_newuserdata(L, sizeof(Vector3));
	p->x = x;
	p->y = y;
	p->z = z;
	create_meta(L);
	lua_setmetatable(L, -2);
}

sll vec3_magnitude(Vector3* self)
{
	return slld2dsqrt( slladd(sllmul(self->x, self->x), slladd(sllmul(self->y, self->y), sllmul(self->z, self->z))) );
}

sll vec3_dot(Vector3 *a, Vector3 *b)
{
	return slladd(sllmul(a->x, b->x), slladd(sllmul(a->y, b->y), sllmul(a->z, b->z)));
}

void vec3_cross(Vector3 * a, Vector3 *b, Vector3 *out)
{
	out->x = sllsub(sllmul(a->y, b->z), sllmul(a->z, b->y));
	out->y = sllsub(sllmul(a->z, b->x), sllmul(a->x, b->z));
	out->z = sllsub(sllmul(a->x, b->y), sllmul(a->y, b->x));
}

void vec3_lerp(Vector3 *a, Vector3 *b, sll tt, Vector3 *out)
{
	tt = clamp_fix(tt, CONST_0, CONST_1);
	out->x = slladd(a->x , sllmul(sllsub(b->x, a->x), tt));
	out->y = slladd(a->y , sllmul(sllsub(b->y, a->y), tt));
	out->z = slladd(a->z , sllmul(sllsub(b->z, a->z), tt));
}

void vec3_set_normalize(Vector3 * self)
{
	sll magnitude = vec3_magnitude(self);
	if (magnitude > CONST_0)
	{
		self->x = slldiv(self->x, magnitude);
		self->y = slldiv(self->y, magnitude);
		self->z = slldiv(self->z, magnitude);
	}
	else
	{
		self->x = CONST_0;
		self->y = CONST_0;
		self->z = CONST_0;
	}
}

static int New(lua_State *L)
{
	lua_settop(L, 4);
	double x = (double)luaL_checknumber(L, 1);
	double y = (double)luaL_checknumber(L, 2);
	double z = (double)luaL_checknumber(L, 3);
	int round_x, round_y, round_z;
	int fn = (int)luaL_optinteger(L, 4, 0);
	if (fn > 6 || fn < 0)
	{
		return luaL_error(L, "转换为定点数只支持0-6位小数精度");
	}
	x = x * _mul[fn];
	y = y * _mul[fn];
	z = z * _mul[fn];
	if (x >= 1000000000 || y >= 1000000000 || z >= 1000000000)
	{
		return luaL_error(L, "有效数字太多转不了，自己看着办");
	}
	round_x = (int)(x + (x > 0 ? 0.5 : -0.5));
	round_y = (int)(y + (y > 0 ? 0.5 : -0.5));
	round_z = (int)(z + (z > 0 ? 0.5 : -0.5));
	push_Vector3(L, int2sll(round_x) / _mul[fn], int2sll(round_y) / _mul[fn], int2sll(round_z) / _mul[fn]);
	return 1;
}

static int NewFromFix(lua_State *L)
{
	check_set_fix(1, x);
	check_set_fix(2, y);
	check_set_fix(3, z);
	push_Vector3(L, *x, *y, *z);
	return 1;	
}

static int NewFromVec2(lua_State *L)
{
	check_set_vec2(1, a);
	push_Vector3(L, a->x, CONST_0, a->y);
	return 1;	
}

static int NormalFromVec3(lua_State *L)
{
	check_set_vec3(1, a);
	push_Vector3(L, sllneg(a->z), CONST_0, a->x);
	return 1;	
}

static int Abs(lua_State *L)
{
	check_set_vec3(1, a);
	push_Vector3(L, sllabs(a->x), sllabs(a->y), sllabs(a->z));
	return 1;	
}

static int get_x(lua_State *L)
{
	check_set_vec3(1, self);
	push_fix(L, self->x);
	return 1;	
}

static int get_y(lua_State *L)
{
	check_set_vec3(1, self);
	push_fix(L, self->y);
	return 1;	
}

static int get_z(lua_State *L)
{
	check_set_vec3(1, self);
	push_fix(L, self->z);
	return 1;	
}

static int Set(lua_State *L)
{
	lua_settop(L, 4);
	check_set_vec3(1, self);
	test_set_fix(2, x);
	test_set_fix(3, y);
	test_set_fix(4, z);
	if (x)
	{
		self->x = *x;
	}
	if (y)
	{
		self->y = *y;
	}
	if (z)
	{
		self->z = *z;
	}
	lua_pop(L, 3);
	return 1;
}

static int SqrMagnitude(lua_State *L)
{
	check_set_vec3(1, self);
	push_fix(L, vec3_dot(self, self));
	return 1;
}

static int Vec2SqrMagnitude(lua_State *L)
{
	check_set_vec3(1, self);
	push_fix(L, slladd(sllmul(self->x, self->x), sllmul(self->z, self->z)));
	return 1;
}

static int Clone(lua_State *L)
{
	check_set_vec3(1, self);
	push_Vector3(L, self->x, self->y, self->z);
	return 1;
}

static int CloneZeroY(lua_State *L)
{
	check_set_vec3(1, self);
	push_Vector3(L, self->x, CONST_0, self->z);
	return 1;
}

static int Distance(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	push_fix(L, slld2dsqrt(slladd(
		sllmul(a->x - b->x, a->x - b->x), 
		slladd(sllmul(a->y - b->y, a->y - b->y),
		sllmul(a->z - b->z, a->z - b->z)))));
	return 1;
}

static int Vec2Distance(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	push_fix(L, slld2dsqrt(slladd(
		sllmul(a->x - b->x, a->x - b->x),
		sllmul(a->z - b->z, a->z - b->z))));
	return 1;
}

static int Vec2SqrDistance(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	push_fix(L, slladd(
		sllmul(a->x - b->x, a->x - b->x),
		sllmul(a->z - b->z, a->z - b->z)));
	return 1;
}

static int Normalize(lua_State *L)
{
	check_set_vec3(1, self);
	Vector3 ret = *self;
	vec3_set_normalize(&ret);
	push_Vector3(L, ret.x, ret.y, ret.z);
	return 1;
}

static int SetNormalize(lua_State *L)
{
	check_set_vec3(1, self);
	vec3_set_normalize(self);
	return 1;
}

static int Dot(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);

	push_fix(L, vec3_dot(a, b));
	return 1;
}

static int Vec2Dot(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);

	push_fix(L, slladd(sllmul(a->x, b->x), sllmul(a->z, b->z)));
	return 1;
}

static int Cross(lua_State *L)
{
	Vector3 out;
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	vec3_cross(a, b, &out);
	push_Vector3(L, out.x, out.y, out.z);
	return 1;
}

static int Vec2Cross(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	push_fix(L, sllsub(sllmul(a->x, b->z), sllmul(a->z, b->x)));
	return 1;
}

static int Angle(lua_State *L)
{
	check_set_vec3(1, from);
	check_set_vec3(2, to);
	vec3_set_normalize(from);
	vec3_set_normalize(to);
	sll d = vec3_dot(from, to);
	d = clamp_fix(d, CONST_neg1, CONST_1);
	push_fix(L, sllmul(sllacos(d), CONST_180_PI));
	return 1;
}

static int Magnitude(lua_State *L)
{
	check_set_vec3(1, self);
	push_fix(L, vec3_magnitude(self));
	return 1;
}

static int Vec2Magnitude(lua_State *L)
{
	check_set_vec3(1, self);
	push_fix(L, slld2dsqrt(slladd(sllmul(self->x, self->x), sllmul(self->z, self->z))));
	return 1;
}

static int SqrDistance(lua_State *L)
{
	check_set_vec2(1, a);
	check_set_vec2(2, b);
	push_fix(L, slladd(sllmul(a->x - b->x, a->x - b->x), sllmul(a->y - b->y, a->y - b->y)));
	return 1;
}

static int Lerp(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	check_set_fix(3, t);
	Vector3 ret;
	vec3_lerp(a, b, *t, &ret);
	push_Vector3(L, ret.x, ret.y, ret.z);
	return 1;
}

static int LerpUnclamped(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	check_set_fix(3, t);
	sll x = slladd(a->x , sllmul(sllsub(b->x, a->x), *t));
	sll y = slladd(a->y , sllmul(sllsub(b->y, a->y), *t));
	sll z = slladd(a->z , sllmul(sllsub(b->z, a->z), *t));
	push_Vector3(L, x, y, z);
	return 1;
}

static int Scale(lua_State *L)
{
	check_set_vec3(1, a);
	check_set_vec3(2, b);
	push_Vector3(L, sllmul(a->x, b->x), sllmul(a->y, b->y), sllmul(a->z, b->z));
	return 1;
}

static int Div(lua_State *L)
{
	check_set_vec3(1, self);
	check_set_fix(2, d);
	push_Vector3(L, slldiv(self->x, *d), slldiv(self->y, *d), slldiv(self->z, *d));
	return 1;
}

static int Mul(lua_State *L)
{
	check_set_vec3(1, self);
	check_set_fix(2, d);
	push_Vector3(L, sllmul(self->x, *d), sllmul(self->y, *d), sllmul(self->z, *d));
	return 1;
}

static int Add(lua_State *L)
{
	check_set_vec3(1, self);
	check_set_vec3(2, b);
	push_Vector3(L, slladd(self->x, b->x), slladd(self->y, b->y), slladd(self->z, b->z));
	return 1;
}

static int AddVec2(lua_State *L)
{
	check_set_vec3(1, self);
	check_set_vec2(2, b);
	self->x += b->x;
	self->z += b->y;
	lua_pop(L, 1);
	return 1;
}

static int AddFix(lua_State *L)
{
	check_set_vec3(1, self);
	check_set_fix(2, b);
	push_Vector3(L, slladd(self->x, *b), slladd(self->y, *b), slladd(self->z, *b));
	return 1;
}

static int Sub(lua_State *L)
{
	check_set_vec3(1, self);
	check_set_vec3(2, b);
	push_Vector3(L, sllsub(self->x, b->x), sllsub(self->y, b->y), sllsub(self->z, b->z));
	return 1;
}

static int SubFix(lua_State *L)
{
	check_set_vec3(1, self);
	check_set_fix(2, b);
	push_Vector3(L, sllsub(self->x, *b), sllsub(self->y, *b), sllsub(self->z, *b));
	return 1;
}

static int Unm(lua_State *L)
{
	check_set_vec3(1, self);
	push_Vector3(L, sllneg(self->x), sllneg(self->y), sllneg(self->z));
	return 1;
}

static int Equal(lua_State *L)
{	
	check_set_vec3(1, self);
	check_set_vec3(2, b);
	lua_pushboolean(L, (self->x == b->x && self->y == b->y && self->z == b->z));
	return 1;
}

static int vec3_tostring(lua_State *L)
{
	char cons[128];
	check_set_vec3(1, self);
	sprintf(cons, "(%.6f,%.6f,%.6f)", (double)self->x / (double)(CONST_1), 
		(double)self->y / (double)(CONST_1), (double)self->z / (double)(CONST_1));
	lua_pushstring(L, cons);
	return 1;
}

static int to_number(lua_State *L)
{
	check_set_vec3(1, self);
	lua_pushnumber(L, (double)(self->x) / (double)(CONST_1) );
	lua_pushnumber(L, (double)(self->y) / (double)(CONST_1) );
	lua_pushnumber(L, (double)(self->z) / (double)(CONST_1) );
	return 3;
}

static const luaL_Reg lua_meta_methods[] = {
	{"__add",   Add},
	{"__sub",   Sub},
	{"__mul",   Mul},
	{"__div",   Div},
	{"__unm",   Unm},
	{"__eq",   Equal},
	{"__tostring",   vec3_tostring},
	{NULL, NULL}
};

static const luaL_Reg lua_vec3_modules[] = {
	{"New",   New},
	{"NewFromFix",   NewFromFix},
	{"NewFromVec2",   NewFromVec2},
	{"NormalFromVec3",   NormalFromVec3},
	{"Abs",   Abs},
	{"AddVec2", AddVec2},
	{"NewFromVec3",   Clone},
	{"Clone",   Clone},
	{"CloneZeroY", CloneZeroY},
	{"get_x",   get_x},
	{"get_y",   get_y},
	{"get_z",   get_z},
	{"Set",   Set},
	{"AddFix",   AddFix},
	{"SubFix",   SubFix},
	{"SqrMagnitude",   SqrMagnitude},
	{"Vec2SqrMagnitude",   Vec2SqrMagnitude},
	{"Distance",   Distance},
	{"SqrDistance",   SqrDistance},
	{"Vec2Distance",   Vec2Distance},
	{"Vec2SqrDistance",   Vec2SqrDistance},
	{"Normalize",   Normalize},
	{"SetNormalize",   SetNormalize},
	{"Dot",   Dot},
	{"Vec2Dot",   Vec2Dot},
	{"Cross",   Cross},
	{"Vec2Cross",   Vec2Cross},
	{"Angle",   Angle},
	{"Magnitude",   Magnitude},
	{"Vec2Magnitude",   Vec2Magnitude},
	{"Lerp",   Lerp},
	{"LerpUnclamped",   LerpUnclamped},
	{"Scale",   Scale},
	{"tonumber",   to_number},
	{NULL, NULL}
};

static void fill_meta(lua_State *L)
{
	luaL_setfuncs(L, lua_meta_methods, 0);
	luaL_newlib(L, lua_vec3_modules);
  	lua_setfield(L, -2, "__index");
}

static void create_meta(lua_State *L)
{
	if(luaL_newmetatable (L, __VECTOR3_META__) != 0)
	{
		fill_meta(L);
	}
}

LUALIB_API int luaopen_fix_vec3(lua_State* L)
{
#ifdef luaL_checkversion
	luaL_checkversion(L);
#endif
#if LUA_VERSION_NUM < 502
    luaL_register(L, "fix_vec3", lua_vec3_modules);
#else
    // 这个傻叉函数，在5.3以前实际是new了一个新的table，然后setfunc，其实没注册到loaded
    luaL_newlib(L, lua_vec3_modules);
#endif
	return 1;
}
