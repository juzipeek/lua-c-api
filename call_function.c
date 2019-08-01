#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/******************************************************************************
 * 函数结构体定义
 ******************************************************************************/
typedef struct function_buf_s {
    char *name;  // 函数名
    int nargs;   // 参数数量
    int nret;    // 返回值数量
    char *chunk; // 函数体
} function_buf;


// 此处 function 不应该使用 function f() ... end 方式编写，写出的函数其实不会被执行
static function_buf 
lua_function[] = {
    {"sayhi", 1, 1,
     "local name = ...\n"
     "return string.format('hi %s!', name)\n"
    },
    {"echo", 1, 1,
     "function echo(msg)\n"
     "    return msg\n"
     "end\n"
     "local msg = ...\n"
     "return echo(msg)\n"
    },
    {NULL, 0, 0,NULL}
};

static int 
sayHi(lua_State *L) {

    function_buf *function = &lua_function[0];
    
    int ret = luaL_loadbuffer(L, function->chunk, strlen(function->chunk), 
                              function->name);
    
    if (ret) {
        return luaL_error(L, "call loadbuffer fail ret:%d, code:%s", 
                          ret, function->chunk);
    }
    
    lua_pushstring(L, "lua");
    ret = lua_pcall(L, 1, 1, 0);
    if (ret != 0) {
        return luaL_error(L, "lua_pcall fail ret:%d %s", 
                          ret, lua_tostring(L, -1));
    }

    // 输出类型
    printf("typename %s\n", luaL_typename(L, -1));

    if (!lua_isstring(L, -1)) {
        return luaL_error(L, "lua_pcall ret not string");
    }
    
    // 函数执行结果在栈中，取出来再放入栈
    const char *msg = lua_tostring(L, -1);
    lua_pop(L, -1);
    lua_pushfstring(L, "sayhi say:%s", msg);
    return 1;
}

/******************************************************************************
 * 通用入口函数
 * 调用时首先将函数名压入栈，之后是其他参数
 ******************************************************************************/
static int
common_entry(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        return luaL_error(L, "first arg should be function name, string type");
    }
    const char * function_name = lua_tostring(L, 1);
    
    function_buf *function = lua_function;
    
    for (;function->name; function++) {
        if (!strcmp(function->name, function_name)) break;
    }
    if (!function->name) {
        return luaL_error(L, "not found:%s function in array", function_name);
    }

    int ret = luaL_loadbuffer(L, function->chunk, strlen(function->chunk), 
                              function->name);
    
    if (ret) {
        return luaL_error(L, "call loadbuffer fail ret:%d, code:%s", 
                          ret, function->chunk);
    }
    
    int i;
    for(i=2; i <= function->nargs+1; i++) {
        lua_pushstring(L, lua_tostring(L, i));
    }

    ret = lua_pcall(L, function->nargs, function->nret, 0);
    if (ret != 0) {
        return luaL_error(L, "lua_pcall fail ret:%d %s", 
                          ret, lua_tostring(L, -1));
    }
    
    int nret = function->nret;
    
    for (i=-1; i >= -nret; i--){
        printf("ret:%d is:%s\n", -i, lua_tostring(L, i));
    }
    
    return nret;
}

static const 
struct luaL_reg call_function_lib[] = {
    {"sayHi", sayHi},
    {"common_entry", common_entry},
    {NULL, NULL}
};

/******************************************************************************
* 注册函数
******************************************************************************/
int 
luaopen_call_function(lua_State *l) {
    luaL_openlib(l, "call_function", call_function_lib, 0);
    return 1;
}
