#include "math-sll.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>

static void create_meta(lua_State *L);

void push_fix(lua_State *L, sll v)
{
	sll* p = lua_newuserdata(L, sizeof(v));
	*p = v;
	create_meta(L);
	lua_setmetatable(L, -2);
}

static int l_tofix(lua_State *L)
{
	lua_settop(L, 2);
	sll* p = luaL_testudata(L, 1, __METATABLE_NAME);
	if(p)
	{
		push_fix(L, *p);
	}
	else
	{
		double val = (double)luaL_checknumber(L, 1);
		int round_val;
		int len = (int)luaL_optinteger(L, 2, 0);
		if (len > 6 || len < 0)
		{
			return luaL_error(L, "转换为定点数只支持0-6位小数精度");
		}
		val = val * _mul[len];
		if (val >= 1000000000)
		{
			return luaL_error(L, "有效数字太多转不了，自己看着办");
		}
		round_val = (int)(val + (val > 0 ? 0.5 : -0.5));
		if (fabs(val - round_val) > 0.2)
		{
			return luaL_error(L, "第%d位小数怎么还有值？", len + 1);
		}
		// 这里可以直接除整数
		push_fix(L, int2sll(round_val) / _mul[len]);
	}
	return 1;
}

/*
	抹去定点数的小数部分，返回一个整形定点数
*/
static int fix_int(lua_State *L)
{
	check_set_fix(1, p);
	push_fix(L, sllint(*p));
	return 1;
}

/*
	抹去定点数的整数部分，返回一个小数定点数
*/
static int fix_frac(lua_State *L)
{
	check_set_fix(1, p);
	push_fix(L, sllfrac(*p));
	return 1;
}

/*
	返回定点数对应的原始integer，定点数本身是个userdata
*/
static int fix_rawvalue(lua_State *L)
{
	check_set_fix(1, p);
	lua_pushinteger(L, *p);
	return 1;
}

static int fix_tostring(lua_State *L)
{
	check_set_fix(1, p);
	lua_pushnumber(L, (double)(*p) / (double)(CONST_1) );
	return 1;
}

static int fix_add(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	push_fix(L, slladd(*a, *b));
	return 1;
}

static int fix_unm(lua_State *L)
{
	check_set_fix(1, p);
	push_fix(L, sllneg(*p));
	return 1;
}

static int fix_sub(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	push_fix(L, sllsub(*a, *b));
	return 1;
}

static int fix_mul(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	push_fix(L, sllmul(*a, *b));
	return 1;
}

static int fix_mul_2n(lua_State *L)
{
	check_set_fix(1, p);
	push_fix(L, sllmul2n(*p, luaL_checkinteger(L, 2)));
	return 1;
}

static int fix_div(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	push_fix(L, slldiv(*a, *b));
	return 1;
}

static int fix_div_2n(lua_State *L)
{
	check_set_fix(1, p);
	push_fix(L, slldiv2n(*p, luaL_checkinteger(L, 2)));
	return 1;
}

static int fix_mod(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	if (*b == 0)
	{
		return luaL_error(L, "mod zero!");
	}
	push_fix(L, *a % *b);
	return 1;
}

static int fix_pow(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	push_fix(L, sllpow(*a, *b));
	return 1;
}

// 倒数
static int fix_inv(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllinv(*a));
	return 1;
}

// 比较函数
static int fix_lt(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	lua_pushboolean(L, *a < *b);
	return 1;
}

static int fix_le(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	lua_pushboolean(L, *a <= *b);
	return 1;
}
static int fix_eq(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	lua_pushboolean(L, *a == *b);
	return 1;
}
static int fix_abs(lua_State *L)
{
	check_set_fix(1, a);
	sll x = *a;
	x = sllabs(x);
	push_fix(L, x);
	return 1;
}
static int fix_floor(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllfloor(*a));
	return 1;
}

static int fix_ceil(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllceil(*a));
	return 1;
}
static int fix_min(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	sll x = *a;
	sll y = *b;
	push_fix(L, min(x, y));
	return 1;	
}
static int fix_max(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	sll x = *a;
	sll y = *b;
	push_fix(L, max(x, y));
	return 1;
}

//-------------- 三角函数 ------------------
static int fix_sin(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllsin(*a));
	return 1;
}
static int fix_cos(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllcos(*a));
	return 1;
}
static int fix_tan(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, slltan(*a));
	return 1;
}
static int fix_sec(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllsec(*a));
	return 1;
}
static int fix_csc(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllcsc(*a));
	return 1;
}
static int fix_cot(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllcot(*a));
	return 1;
}
static int fix_sinh(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllsinh(*a));
	return 1;
}
static int fix_cosh(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllcosh(*a));
	return 1;
}
static int fix_tanh(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, slltanh(*a));
	return 1;
}
static int fix_sech(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllsech(*a));
	return 1;
}
static int fix_csch(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllcsch(*a));
	return 1;
}
static int fix_coth(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllcoth(*a));
	return 1;
}
// 反三角函数
static int fix_asin(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllasin(*a));
	return 1;	
}
static int fix_acos(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllacos(*a));
	return 1;
}
static int fix_atan(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllatan(*a));
	return 1;
}
static int fix_deg(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllmul(*a, CONST_180_PI));
	return 1;
}
static int fix_rad(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllmul(*a, CONST_PI_180));
	return 1;
}

static int fix_sqrt(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllsqrt(*a));
	return 1;
}

static int fix_sqrt_ex(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, slld2dsqrt(*a));
	return 1;
}

static int fix_exp(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, sllexp(*a));
	return 1;	
}

static int fix_log(lua_State *L)
{
	check_set_fix(1, a);
	push_fix(L, slllog(*a));
	return 1;
}

sll clamp_fix(sll a, sll b, sll c)
{
	if (a < b)
	{
		a = b;
	}
	else if(a > c)
	{
		a = c;
	}
	return a;
}

static int fix_clamp(lua_State *L)
{
	check_set_fix(1, a);
	check_set_fix(2, b);
	check_set_fix(3, c);
	push_fix(L, clamp_fix(*a, *b, *c));
	return 1;
}
// For internal use before precompile
/*static int l_output_predefined_values(lua_State *L)
{
	output_predefined_values();
	lua_pushnil(L);
	return 1;
}*/

const luaL_Reg lua_fixmath_meta_methods[] = {
	{"__add",   fix_add},
	{"__sub",   fix_sub},
	{"__mul",   fix_mul},
	{"__div",   fix_div},
	{"__mod",   fix_mod},
	{"__pow",   fix_pow},
	{"__unm",   fix_unm},
	{"__lt",   fix_lt},
	{"__le",   fix_le},
	{"__eq",   fix_eq},
	{"__tostring",   fix_tostring},
	{NULL, NULL}
};

const luaL_Reg lua_fixmath_modules[] = {
	{"tofix",   l_tofix},
	{"tostring",   fix_tostring},
	{"tonumber",   fix_tostring},
	{"rawvalue",   fix_rawvalue},
	{"int",   fix_int},
	{"frac",   fix_frac},
	{"abs",   fix_abs},
	{"floor",   fix_floor},
	{"ceil",   fix_ceil},
	{"mul_2n",   fix_mul_2n},  //其实就是位移，比正常的乘法快一点
	{"div_2n",   fix_div_2n},
	{"inv",   fix_inv},
	{"min",   fix_min},
	{"max",   fix_max},
	{"clamp",	fix_clamp},
// begin 三角函数
	{"sin",   fix_sin},
	{"cos",   fix_cos},
	{"tan",   fix_tan},
	{"sec",   fix_sec},
	{"csc",   fix_csc},
	{"cot",   fix_cot},
	{"sinh",   fix_sinh},
	{"cosh",   fix_cosh},
	{"tanh",   fix_tanh},
	{"sech",   fix_sech},
	{"csch",   fix_csch},
	{"coth",   fix_coth},
	{"asin",   fix_asin},
	{"acos",   fix_acos},
	{"atan",   fix_atan},
// end 三角函数

	{"deg",   fix_deg},
	{"rad",   fix_rad},
	{"sqrt",   fix_sqrt},
	{"sqrt_ex",   fix_sqrt_ex},	
	{"exp",   fix_exp},
	{"log",   fix_log},
	//{"output_predefined_values", l_output_predefined_values}, // For internal use before precompile
	{NULL, NULL}
};

static void fill_meta(lua_State *L)
{
	luaL_setfuncs(L, lua_fixmath_meta_methods, 0);
	luaL_newlib(L, lua_fixmath_modules);
  	lua_setfield(L, -2, "__index");
}

static void create_meta(lua_State *L)
{
	if(luaL_newmetatable (L, __METATABLE_NAME) != 0)
	{
		fill_meta(L);
	}
}

#define push_const(x, y) push_fix(L, x); lua_setfield(L, -2, #y)
static void fill_const(lua_State *L)
{
	push_const(CONST_0, zero);
	push_const(CONST_1, one);
	push_const(CONST_2, two);
	push_const(CONST_E, e);
	push_const(CONST_1_2, half);
	push_const(CONST_PI, pi);
	push_const(CONST_MAX, huge);
	push_const(CONST_MIN, tiny);
}

LUALIB_API int luaopen_fixmath(lua_State* L)
{
#ifdef luaL_checkversion
	luaL_checkversion(L);
#endif
#if LUA_VERSION_NUM < 502
    luaL_register(L, "fixmath", lua_fixmath_modules);
#else
    // 这个傻叉函数，在5.3以前实际是new了一个新的table，然后setfunc，其实没注册到loaded
    luaL_newlib(L, lua_fixmath_modules);
#endif
    fill_const(L);
	return 1;
}