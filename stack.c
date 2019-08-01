#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/******************************************************************************
 * sayhi 函数
 * 清空栈，并将 hi 字符串压入栈顶
 ******************************************************************************/
static int 
sayhi(lua_State *L) {
    lua_settop(L, 0);
    lua_pushstring(L, "hi");
    return 1;
}

/******************************************************************************
 * echo 函数会返回所有输入参数
 ******************************************************************************/
static int
echo(lua_State *L) {
    int num = lua_gettop(L);
    return num;
}

static const 
struct luaL_reg stack_lib[] = {
    {"sayhi", sayhi},
    {"echo", echo},
    {NULL, NULL}
};

/******************************************************************************
 * 注册函数
 ******************************************************************************/
int 
luaopen_stack(lua_State *l) {
    luaL_openlib(l, "stack", stack_lib, 0);
    return 1;
}
