# lua c api

---
> 2019/07/13
> zcj
---

## 概述

本项目是一个测试 `demo`，在阅读官方文档时进行编码测试加深印象。

## 一 newCounter

`newCounter` 在 `C` 中创建一个闭包，返回给 `lua` 使用。

```lua
-- use package.loadlib or require to dynamic library
-- l,e = package.loadlib("./counter.so", "luaopen_counter")
require "counter"
c1 = counter.newCounter()
print(type(c1)) --> function
c2 = counter.newCounter()
print(c1()) --> 1
print(c1()) --> 2
print(c2()) --> 1
```

*编译使用的 lua 版本一定要与使用的 lua 解释器相同。在测试时因为使用解释器不同一直报”undefined symbol: luaopen_counter“，比较好的方式是下载 lua 源码，进行编译，然后使用 lua 源码编译结果进行链接、执行。*

### 说明

```c
#define luaI_openlib luaL_openLib

typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;

/* luaL_register 实际调用的也是 luaI_openlib 函数 */
LUALIB_API void 
(luaL_register) (lua_State *L, const char *libname, const luaL_Reg *l) {
  luaI_openlib(L, libname, l, 0);
}

LUALIB_API void 
luaI_openlib (lua_State *L, const char *libname, const luaL_Reg *l, int nup) {
  if (libname) {
    int size = libsize(l);
    /* check whether lib already exists */
    luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", 1);
    lua_getfield(L, -1, libname);  /* get _LOADED[libname] */
    if (!lua_istable(L, -1)) {  /* not found? */
      lua_pop(L, 1);  /* remove previous result */
      /* try global variable (and create one if it does not exist) */
      if (luaL_findtable(L, LUA_GLOBALSINDEX, libname, size) != NULL)
        luaL_error(L, "name conflict for module " LUA_QS, libname);
      lua_pushvalue(L, -1);
      lua_setfield(L, -3, libname);  /* _LOADED[libname] = new table */
    }
    lua_remove(L, -2);  /* remove _LOADED table */
    lua_insert(L, -(nup+1));  /* move library table to below upvalues */
  }
  for (; l->name; l++) {
    int i;
    for (i=0; i<nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);
    lua_setfield(L, -(nup+2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}
```

`luaL_register\luaI_openlib\luaL_openlib` 三个函数都提供了注册函数列表到 `table` 的功能。`luaL_register` 是通过调用 `luaI_openlib` 函数来实现，提供了无闭包函数的功能。`luaI_openlib` 与 `luaL_openlib` 是同一个函数都会将函数注册到 `table` 中，`nup` 指定”上值“数量。
`luaI_openlib` 思路是操作栈，将需要注册的函数保存到栈底的 `table` 中，函数执行完毕后栈底保留原先 `table`。

## 二 stack

栈操作示例。

- sayhi 函数
    清空栈，并将 "hi" 压入栈。
- echo 函数
    返回所有输入参数

## 三 private_cfg

table 操作示例。

### `set_channel_key`

`set_channel_key` 函数接收一个 table 作为参数，从 table 中获取 channel_name 的值作为 key，之后从动态库的全局渠道秘钥表中获取相应渠道的秘钥，并在 table 中添加 channel_key 条目。


执行示例：

```bash
$ lua-5.1
Lua 5.1.5  Copyright (C) 1994-2012 Lua.org, PUC-Rio
> requrie "private_cfg"
stdin:1: attempt to call global 'requrie' (a nil value)
stack traceback:
	stdin:1: in main chunk
	[C]: ?
> require "private_cfg"
> tb = {channel_name = 'mobile', channel_key = 'no'}
> private_cfg.set_channel_key(tb)
> print(tb['channel_key'])
mobile key
```

### `do_init_lib`

向 lua_State 中注入全局变量。

调用示例：
```lua
require "private_cfg"
private_cfg.init_lib()

for k,v in pairs(_G) do
    print(k..":"..type(v))
end

-- 能看到包含如下输出
-- pc_online:table
-- mobile_online:table
-- mobile_test:table

for k,v in pairs(pc_online) do
    print(k.."->"..v)
end

-- 输出如下
-- channel_version->v7.1
-- channel_env->product
-- channel_name->pc
```

函数说明：

```c
void lua_setglobal (lua_State *L, const char *name);
#define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, s)
```

## 四 call_function

很多情况下有这种需求：在特定的框架中针对不同的业务做少量修改。使用 `C/C++` 开发稳定的框架，调用针对不同业务开发的 `Lua` 函数可以实现需求。其实从 `C/C++` 调用 `Lua` 函数非常简单，**调用时将 `Lua` 函数压入栈、将函数参数压入栈，调用 `lua_pcall` 完成调用；调用后从栈获得调用函数返回结果**。

程序中有两个示例，`sayHi` 函数接收一个 `string` 参数并返回一个 `string` 参数；`common_entry` 函数可以根据函数名选择相应的函数体进行执行，更加灵活。

**在函数定义中需要注意，`luaL_loadbuffer` 系列函数会将字符串当做程序段载入并放在栈顶，但是并未执行函数。如果载入的代码段是 `function f()…end` 格式，使用 `lua_pcall` 调用代码段时函数 `f` 仅做了定义，未执行函数体。**

### 调用示例

```bash
$ /usr/local/lua5.1.5/bin/lua -e "require 'call_function'; ret = call_function.sayHi('lua'); print(ret)"
typename string
sayhi say:hi lua!

$ /usr/local/lua5.1.5/bin/lua -e "require 'call_function'; ret = call_function.common_entry('echo','lua is cool'); print(ret)"
ret:1 is:lua is cool
lua is cool
```

### `luaL_error`

```c
int luaL_error (lua_State *L, const char *fmt, ...);
```

触发错误，并使用 `fmt` 与后续参数格式化输出错误消息字符串。`luaL_error` 函数并不会退出，通常的是否用法为：

```c
return luaL_error(args);
```

## 五 userdata

在 `Lua` 中使用 `userdata` 表示 `C` 中的复杂数据类型（其实是一块内存区域），对于 `uerdata` 没有预定义的操作。`light userdata` 表示指针类型数据，并不需要创建（它是指针值），同时，**`light userdata` 不被 `gc` 管理**。`ligth userdata` 的主要用途是用户自己管理内存，避免 `gc` 管理内存。

###  使用 `userdata` 实现数组

在 `lua` 中使用 `table` 作为数组，在数组变大时会耗费非常多的内存，使用 `userdata` 能够降低内存使用。`userdata` 可以有 `metatable`，对 `userdata` 类型进行识别（判断 `C` 类型是否正确）增加元方法。

### 调用示例

```bash
$ /usr/local/lua5.1.5/bin/lua -e "require 'userdata'; arr = array.new(20); print(type(arr)); array.set(arr,1,20);print('arr[1]=',array.get(arr,1));print('size:',array.size(arr))"
userdata
arr[1]=	20
size:	20
```

### 添加元方法

在创建元表时可以在元表中添加元方法，方便使用 `Lua` 的 `__index`、`__newindex` 方法。修改 `luaopen_userdata` 方法实现：

```c
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
```

**在这里的栈操作要仔细**

### 调用示例

```bash
$ /usr/local/lua5.1.5/bin/lua -e "require 'userdata'; arr = array.new(20); print(type(arr)); arr[1]=20;print('arr[1]=',arr[1])"
userdata
arr[1]=	20
```

### 1. `lua_newuserdata`

```c
void *lua_newuserdata (lua_State *L, size_t size);
```

创建一块 `size` 大小的内存区域，将其压入栈顶并返回内存地址指针。

### 2. `luaL_checkudata`

```c
void *luaL_checkudata (lua_State *L, int narg, const char *tname);
```

检查函数的第 `narg` 参数是否是 `tname` 类型的 `userdata`。`luaL_checkudata` 首先将 `narg` 转换为 `userdata` ，然后获得元表；同时，从注册表中根据 `tname` 获得元表，两者相互比较如果不同触发错误并返回 `NULL`，如果相同则返回 `narg` 指向的 `userdata`。

*可以看 `luaL_checkudata` 代码实现，非常简单。*

### 3. `luaL_newmetatable`

```c
int luaL_newmetatable (lua_State *L, const char *tname);
```

创建一个新的可以作为 `userdata` 类型元表的 `table`，并使用 `tname` 作为 `key` 存储在注册表中。如果注册表中已经存在 `tname` 类型的值，返回值为 `0`。

### 4. `luaL_getmetatable`

```c
void luaL_getmetatable (lua_State *L, const char *tname);
```

将注册表中与名称 `tname` 关联的元表压入栈中。

### 5. `lua_setmetatable`

```c
int lua_setmetatable (lua_State *L, int index);
```

从栈顶弹出一个 `table` 并将其设置为栈中 `index` 索引出的值的元表。
