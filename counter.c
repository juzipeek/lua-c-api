#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

static int 
counter(lua_State *L) {
  // lua_upvalueindex 同样适用伪索引技术，取 counter 函数的第一个 upvale
  double val = lua_tonumber(L, lua_upvalueindex(1));
  lua_pushnumber(L, ++val); // 栈顶值为 upvalue 值加 1
  lua_pushvalue(L, -1); // 栈顶增加一个相同值，用来更新 upvalue 值
  lua_replace(L, lua_upvalueindex(1)); // 更新 upvalue 值
  return 1;
}

static int 
newCounter(lua_State *L) {
  lua_pushnumber(L, 0); // 栈顶增加数字，后续作为闭包的值
  // 创建闭包
  // &counter 是闭包基函数；注意是函数名取地址
  // 1 告诉闭包 upvalue 的数目
  lua_pushcclosure(L, &counter, 1);
  return 1;
}

static const 
struct luaL_reg counter_lib[] = {
    {"newCounter", newCounter},
    {NULL, NULL}
};

/******************************************************************************
 * 注册函数
 * 注意，在 lua 中 require 时，必须与 luaopen_xxxx 相同
 ******************************************************************************/
int 
luaopen_counter(lua_State *l) {
    luaL_openlib(l, "counter", counter_lib, 0);
    return 1;
}
