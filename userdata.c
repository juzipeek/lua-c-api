#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

typedef struct NumArray {
    int size;
    double values[1];  /* variable part */
} NumArray;

/******************************************************************************
* 创建 array
******************************************************************************/
static int 
newarray(lua_State *L) {
    int n = luaL_checkint(L, 1);
    size_t nbytes = sizeof(NumArray) + (n - 1)*sizeof(double);
    NumArray *a = (NumArray *)lua_newuserdata(L, nbytes);
    
    luaL_getmetatable(L, "LuaBook.array");
    lua_setmetatable(L, -2);
    
    a->size = n;
    return 1;  /* new userdatum is already on the stack */
}

static NumArray *
checkarray(lua_State *L) {
    void *ud = luaL_checkudata(L, 1, "LuaBook.array");
    luaL_argcheck(L, ud != NULL, 1, "`array' expected");
    return (NumArray *)ud;
}

/******************************************************************************
* 更新 array 中内容
* @param  userdata    array
* @param  index
* @para   value
******************************************************************************/
static int 
setarray(lua_State *L) {
    //NumArray *a = (NumArray *)lua_touserdata(L, 1);
    NumArray *a = checkarray(L);
    luaL_argcheck(L, a != NULL, 1, "`array' expected");
    
    int index = luaL_checkint(L, 2);
    luaL_argcheck(L, 1 <= index && index <= a->size, 2,
                  "index out of range");
    
    double value = luaL_checknumber(L, 3);
    
    a->values[index-1] = value;
    return 0;
}

/******************************************************************************
* 获得 array 中内容
* @param  userdata    array
* @param  index
******************************************************************************/
static int 
getarray(lua_State *L) {
    //NumArray *a = (NumArray *)lua_touserdata(L, 1);
    NumArray *a = checkarray(L);
    luaL_argcheck(L, a != NULL, 1, "`array' expected");
    
    int index = luaL_checkint(L, 2);
    luaL_argcheck(L, 1 <= index && index <= a->size, 2,
                  "index out of range");
    
    lua_pushnumber(L, a->values[index-1]);
    return 1;
}

/******************************************************************************
* 获得 array 大小
* @param  userdata    array
******************************************************************************/
static int 
getsize(lua_State *L) {
    // NumArray *a = (NumArray *)lua_touserdata(L, 1);
    NumArray *a = checkarray(L);
    
    luaL_argcheck(L, a != NULL, 1, "`array' expected");
    lua_pushnumber(L, a->size);
    return 1;
}

static const 
struct luaL_reg array_lib[] = {
    {"new", newarray},
    {"set", setarray},
    {"get", getarray},
    {"size", getsize},
    {NULL, NULL}
};

/******************************************************************************
* 注册函数
******************************************************************************/
int 
luaopen_userdata(lua_State *l) {
    // 创建 metatable，使用 array.new 创建的数组元表是同一个元表
    luaL_newmetatable(l, "LuaBook.array");
    luaL_openlib(l, "array", array_lib, 0);    
    
    /* now the stack has the metatable at index 1 and
       `array' at index 2 */
    lua_pushstring(l, "__index");
    lua_pushstring(l, "get");
    lua_gettable(l, -3);  /* get array.get on stack top*/
    lua_settable(l, -4);  /* metatable.__index = array.get */
    
    lua_pushstring(l, "__newindex");
    lua_pushstring(l, "set");
    lua_gettable(l, -3); /* get array.set */
    lua_settable(l, -4); /* metatable.__newindex = array.set */
    
    return 0;
}
