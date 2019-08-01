#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


typedef struct channel_key_s {
    char *channel;
    char *key;
} channel_key;


static channel_key 
g_channel_keys[] = {
    {"mobile", "mobile key"},
    {"pc", "pc key"},
    {NULL, NULL},
};

/*******************************************************************************
 * 从全局渠道秘钥表获取秘钥
 ******************************************************************************/
static char *
get_key(char *channel_name) {
    char *key = NULL;
    
    channel_key *p = g_channel_keys;
    for (;p->channel;p++) {
        if (!strcmp(p->channel, channel_name)) {
            key = p->key;
            break;
        }
    }

    return key;
}


/*******************************************************************************
 * do_set_channel_key 
 * 函数接收一个 table 作为参数，从 table 中获取 channel_name 的值作为 key，
 * 之后从全局渠道秘钥表中获取相应渠道的秘钥，并在 table 中添加 channel_key 条目。
 ******************************************************************************/
static int
do_set_channel_key(lua_State *L) {
    // 参数必须是 table
    if (!lua_istable(L, -1)) {
        return luaL_error(L, "type error, should use table as param!");
    }
    
    // 将 table 的 key `channel` 压入栈顶
    lua_pushstring(L, "channel_name");
    // 从 table 中获取 channel 字段，并其放到栈顶
    // 注意：此时栈顶是 channel 值，栈顶第二个元素是 table。
    //      原先栈顶的 "channel" 字符串已经被使用并移出栈。
    lua_gettable(L, -2);
    if (!lua_isstring(L, -1)) {
        return luaL_error(L, "channel type error, should use string as param!");
    }
    
    // 取出 channel 名
    char *channel_name = (char *)lua_tostring(L, -1);

    // 将栈顶的 channel 值出栈，此时栈顶元素是 table
    lua_pop(L, 1);
    
    char *key = get_key(channel_name);
    
    lua_pushstring(L, "channel_key");
    lua_pushstring(L, key);
    lua_settable(L, -3);
    
    return 0;
}

/*******************************************************************************
 * 渠道信息结构体
 ******************************************************************************/
typedef struct channels_info_s {
    char *name;
    char *channel_name;
    char *channel_version;
    char *channel_env;
} channel_info;

channel_info g_channels[] = {
    {"mobile_test"  , "mobile1", "v5.1", "test"},
    {"mobile_online", "mobile1", "v4.8", "product"},
    {"pc_online"    , "pc"     , "v7.1", "product"},
    {NULL    },
};

/*******************************************************************************
 * 注入一个渠道信息
 ******************************************************************************/
static void
inject_one_channel(lua_State *L, channel_info *info) {
    // create new table
    lua_newtable(L);
    
    // add channel_name field
    lua_pushstring(L, "channel_name");
    lua_pushstring(L, info->channel_name);
    lua_settable(L, -3);
    
    // add channel_version field
    lua_pushstring(L, "channel_version");
    lua_pushstring(L, info->channel_version);
    lua_settable(L, -3);

    // add channel_env field
    lua_pushstring(L, "channel_env");
    lua_pushstring(L, info->channel_env);
    lua_settable(L, -3);
    
    // add table as L's global var, var name is info->name
    // 在 L 中添加一个全局变量 info->name，其值为栈顶元素。
    // 注意：栈顶元素会出栈。
    lua_setglobal(L, info->name);
}

/*******************************************************************************
 * do_init_lib
 * 给 L 注入全局变量
 ******************************************************************************/
static int
do_init_lib(lua_State *L) {
    channel_info *p = g_channels;
    for (;p->name; p++) {
        inject_one_channel(L, p);
    }

    return 0;
}

static const 
struct luaL_reg private_cfg[] = {
    {"init_lib", do_init_lib},
    {"set_channel_key", do_set_channel_key},
    {NULL, NULL}
};

/******************************************************************************
* 注册函数
******************************************************************************/
int 
luaopen_private_cfg(lua_State *l) {
    luaL_openlib(l, "private_cfg", private_cfg, 0);
    return 1;
}
