#include "math-sll.h"

static void create_meta(lua_State *L);

void push_Vector2(lua_State *L, sll x, sll y)
{
	Vector2* p = lua_newuserdata(L, sizeof(Vector2));
	p->x = x;
	p->y = y;
	create_meta(L);
	lua_setmetatable(L, -2);
}

sll vec2_dot(Vector2 *a, Vector2 *b)
{
	return slladd(sllmul(a->x, b->x), sllmul(a->y, b->y));
}

sll vec2_sqrmagnitude(Vector2 *self)
{
	return slladd(sllmul(self->x, self->x), sllmul(self->y, self->y));
}

sll vec2_magnitude(Vector2 *self)
{
	return slld2dsqrt(vec2_sqrmagnitude(self));
}

sll vec2_cross(Vector2 *a, Vector2 *b)
{
	return sllsub(sllmul(a->x, b->y), sllmul(a->y, b->x));
}

void vec2_lerp(Vector2 *a, Vector2 *b, sll tt, Vector2 *out)
{
	tt = clamp_fix(tt, CONST_0, CONST_1);
	out->x = slladd(a->x , sllmul(sllsub(b->x, a->x), tt));
	out->y = slladd(a->y , sllmul(sllsub(b->y, a->y), tt));
}

void vec2_set_normalize(Vector2 * self)
{
	sll magnitude = vec2_magnitude(self);
	if (magnitude > CONST_0)
	{
		self->x = slldiv(self->x, magnitude);
		self->y = slldiv(self->y, magnitude);
	}
	else
	{
		self->x = CONST_0;
		self->y = CONST_0;
	}
}

static int New(lua_State *L)
{
	lua_settop(L, 3);
	double x = (double)luaL_checknumber(L, 1);
	double y = (double)luaL_checknumber(L, 2);
	int round_x, round_y;
	int fn = (int)luaL_optinteger(L, 3, 0);
	if (fn > 6 || fn < 0)
	{
		return luaL_error(L, "转换为定点数只支持0-6位小数精度");
	}
	x = x * _mul[fn];
	y = y * _mul[fn];
	if (x >= 1000000000 || y >= 1000000000)
	{
		return luaL_error(L, "有效数字太多转不了，自己看着办");
	}
	round_x = (int)(x + (x > 0 ? 0.5 : -0.5));
	round_y = (int)(y + (y > 0 ? 0.5 : -0.5));
	push_Vector2(L, int2sll(round_x) / _mul[fn], int2sll(round_y) / _mul[fn]);
	return 1;
}

static int NewFromFix(lua_State *L)
{
	check_set_fix(1, x);
	check_set_fix(2, y);
	push_Vector2(L, *x, *y);
	return 1;	
}

static int NewFromVec3(lua_State *L)
{
	check_set_vec3(1, self);
	push_Vector2(L, self->x, self->z);
	return 1;	
}

static int NormalFromVec3(lua_State *L)
{
	check_set_vec3(1, self);
	push_Vector2(L, sllneg(self->z), self->x);
	return 1;	
}

static int NormalFromVec2(lua_State *L)
{
	check_set_vec2(1, self);
	push_Vector2(L, sllneg(self->y), self->x);
	return 1;	
}

static int get_x(lua_State *L)
{
	check_set_vec2(1, self);
	push_fix(L, self->x);
	return 1;	
}

static int get_y(lua_State *L)
{
	check_set_vec2(1, self);
	push_fix(L, self->y);
	return 1;	
}

static int Set(lua_State *L)
{
	lua_settop(L, 3);
	check_set_vec2(1, self);
	test_set_fix(2, x);
	test_set_fix(3, y);

	if (x)
	{
		self->x = *x;
	}
	if (y)
	{
		self->y = *y;
	}
	lua_pop(L, 2);
	return 1;
}

static int SqrMagnitude(lua_State *L)
{
	check_set_vec2(1, self);
	push_fix(L, vec2_sqrmagnitude(self));
	return 1;
}

static int Clone(lua_State *L)
{
	check_set_vec2(1, self);
	push_Vector2(L, self->x, self->y);
	return 1;
}

static int Normalize(lua_State *L)
{
	check_set_vec2(1, self);
	Vector2 ret = *self;
	vec2_set_normalize(&ret);
	push_Vector2(L, ret.x, ret.y);
	return 1;
}

static int SetNormalize(lua_State *L)
{
	check_set_vec2(1, self);
	vec2_set_normalize(self);
	return 1;
}

static int Dot(lua_State *L)
{
	check_set_vec2(1, a);
	check_set_vec2(2, b);

	push_fix(L, vec2_dot(a, b));
	return 1;
}

static int Cross(lua_State *L)
{
	check_set_vec2(1, a);
	check_set_vec2(2, b);
	push_fix(L, vec2_cross(a, b));
	return 1;
}

static int Angle(lua_State *L)
{
	check_set_vec2(1, from);
	check_set_vec2(2, to);
	Vector2 _from = *from;
	Vector2 _to = *to;
	vec2_set_normalize(&_from);
	vec2_set_normalize(&_to);
	sll d = slladd(sllmul(_from.x, _to.x), sllmul(_from.y, _to.y));
	d = clamp_fix(d, CONST_neg1, CONST_1);
	push_fix(L, sllmul(sllacos(d), CONST_180_PI));
	return 1;
}

static int Magnitude(lua_State *L)
{
	check_set_vec2(1, self);
	push_fix(L, vec2_magnitude(self));
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
	check_set_vec2(1, a);
	check_set_vec2(2, b);
	check_set_fix(3, t);
	Vector2 ret;
	vec2_lerp(a, b, *t, &ret);
	push_Vector2(L, ret.x, ret.y);
	return 1;
}

static int LerpUnclamped(lua_State *L)
{
	check_set_vec2(1, a);
	check_set_vec2(2, b);
	check_set_fix(3, t);
	sll x = slladd(a->x , sllmul(sllsub(b->x, a->x), *t));
	sll y = slladd(a->y , sllmul(sllsub(b->y, a->y), *t));
	push_Vector2(L, x, y);
	return 1;
}

static int Max(lua_State *L)
{
	int top = lua_gettop(L);
	sll max_x = CONST_MIN;
	sll max_y = CONST_MIN;
	for (int i = 1; i <= top; i++)
	{
		check_set_vec2(i, a);
		max_x = max(max_x, a->x);
		max_y = max(max_y, a->y);
	}
	push_Vector2(L, max_x, max_y);
	return 1;
}

static int Min(lua_State *L)
{
	int top = lua_gettop(L);
	sll min_x = CONST_MAX;
	sll min_y = CONST_MAX;
	for (int i = 1; i <= top; i++)
	{
		check_set_vec2(i, a);
		min_x = min(min_x, a->x);
		min_y = min(min_y, a->y);
	}
	push_Vector2(L, min_x, min_y);
	return 1;
}

static int Abs(lua_State *L)
{
	check_set_vec2(1, a);
	push_Vector2(L, sllabs(a->x), sllabs(a->y));
	return 1;	
}

static int Scale(lua_State *L)
{
	check_set_vec2(1, a);
	check_set_vec2(2, b);
	push_Vector2(L, sllmul(a->x, b->x), sllmul(a->y, b->y));
	return 1;
}

static int Div(lua_State *L)
{
	check_set_vec2(1, self);
	check_set_fix(2, d);
	push_Vector2(L, slldiv(self->x, *d), slldiv(self->y, *d));
	return 1;
}

static int Mul(lua_State *L)
{
	check_set_vec2(1, self);
	check_set_fix(2, d);
	push_Vector2(L, sllmul(self->x, *d), sllmul(self->y, *d));
	return 1;
}

static int Add(lua_State *L)
{
	check_set_vec2(1, self);
	check_set_vec2(2, b);
	push_Vector2(L, slladd(self->x, b->x), slladd(self->y, b->y));
	return 1;
}

static int AddFix(lua_State *L)
{
	check_set_vec2(1, self);
	check_set_fix(2, b);
	push_Vector2(L, slladd(self->x, *b), slladd(self->y, *b));
	return 1;
}

static int Sub(lua_State *L)
{
	check_set_vec2(1, self);
	check_set_vec2(2, b);
	push_Vector2(L, sllsub(self->x, b->x), sllsub(self->y, b->y));
	return 1;
}

static int SubFix(lua_State *L)
{
	check_set_vec2(1, self);
	check_set_fix(2, b);
	push_Vector2(L, sllsub(self->x, *b), sllsub(self->y, *b));
	return 1;
}

static int Unm(lua_State *L)
{
	check_set_vec2(1, self);
	push_Vector2(L, sllneg(self->x), sllneg(self->y));
	return 1;
}

static int Equal(lua_State *L)
{	
	check_set_vec2(1, self);
	check_set_vec2(2, b);
	lua_pushboolean(L, (self->x == b->x && self->y == b->y));
	return 1;
}

static int vec2_tostring(lua_State *L)
{
	char cons[64];
	check_set_vec2(1, self);
	sprintf(cons, "(%.6f,%.6f)", (double)self->x / (double)(CONST_1), (double)self->y / (double)(CONST_1));
	lua_pushstring(L, cons);
	return 1;
}

static int to_number(lua_State *L)
{
	check_set_vec2(1, self);
	lua_pushnumber(L, (double)(self->x) / (double)(CONST_1) );
	lua_pushnumber(L, (double)(self->y) / (double)(CONST_1) );
	return 2;
}

static const luaL_Reg lua_meta_methods[] = {
	{"__add",   Add},
	{"__sub",   Sub},
	{"__mul",   Mul},
	{"__div",   Div},
	{"__unm",   Unm},
	{"__eq",   Equal},
	{"__tostring",   vec2_tostring},
	{NULL, NULL}
};

static const luaL_Reg lua_vec2_modules[] = {
	{"New",   New},
	{"get_x", get_x},
	{"get_y", get_y},
	{"Set", Set},
	{"Abs", Abs},
	{"AddFix",   AddFix},
	{"SubFix",   SubFix},
	{"NewFromFix",   NewFromFix},
	{"NewFromVec3",   NewFromVec3},
	{"NormalFromVec3",   NormalFromVec3},
	{"NormalFromVec2",   NormalFromVec2},
	{"SqrMagnitude",   SqrMagnitude},
	{"Clone",   Clone},
	{"Normalize",   Normalize},
	{"SetNormalize",   SetNormalize},
	{"Dot",   Dot},
	{"Cross",   Cross},
	{"Angle",   Angle},
	{"Magnitude",   Magnitude},
	{"Lerp",   Lerp},
	{"LerpUnclamped",   LerpUnclamped},
	{"Min",   Min},
	{"Max",   Max},
	{"Scale",   Scale},
	{"tonumber",   to_number},
	{NULL, NULL}
};

static void fill_meta(lua_State *L)
{
	luaL_setfuncs(L, lua_meta_methods, 0);
	luaL_newlib(L, lua_vec2_modules);
  	lua_setfield(L, -2, "__index");
}

static void create_meta(lua_State *L)
{
	if(luaL_newmetatable (L, __VECTOR2_META__) != 0)
	{
		fill_meta(L);
	}
}

LUALIB_API int luaopen_fix_vec2(lua_State* L)
{
#ifdef luaL_checkversion
	luaL_checkversion(L);
#endif
#if LUA_VERSION_NUM < 502
    luaL_register(L, "fix_vec2", lua_vec2_modules);
#else
    // 这个傻叉函数，在5.3以前实际是new了一个新的table，然后setfunc，其实没注册到loaded
    luaL_newlib(L, lua_vec2_modules);
#endif
	return 1;
}
