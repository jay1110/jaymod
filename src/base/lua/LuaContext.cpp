#include <base/public.h>

namespace lua {

LuaContext::LuaContext(bool loadStdLibs)
    : state(NULL)
    , errorMsg("")
{
    state = luaL_newstate();
    if (!state) {
        errorMsg = "Failed to create Lua state";
        return;
    }
    if (loadStdLibs) {
        luaL_openlibs(state);
    }
}

LuaContext::~LuaContext()
{
    if (state) {
        lua_close(state);
        state = NULL;
    }
}

bool LuaContext::doString(const string& code)
{
    if (!state) {
        errorMsg = "Lua state not initialized";
        return false;
    }

    int result = luaL_dostring(state, code.c_str());
    if (result != 0) {
        const char* errStr = lua_tostring(state, -1);
        errorMsg = errStr ? errStr : "Unknown error";
        lua_pop(state, 1);
        return false;
    }
    errorMsg = "";
    return true;
}

bool LuaContext::doFile(const string& filename)
{
    if (!state) {
        errorMsg = "Lua state not initialized";
        return false;
    }

    int result = luaL_dofile(state, filename.c_str());
    if (result != 0) {
        const char* errStr = lua_tostring(state, -1);
        errorMsg = errStr ? errStr : "Unknown error";
        lua_pop(state, 1);
        return false;
    }
    errorMsg = "";
    return true;
}

const string& LuaContext::getErrorMsg() const
{
    return errorMsg;
}

lua_State* LuaContext::getState() const
{
    return state;
}

bool LuaContext::isValid() const
{
    return state != NULL;
}

void LuaContext::registerFunction(const char* name, lua_CFunction func)
{
    if (state && name && func) {
        lua_register(state, name, func);
    }
}

int LuaContext::getGlobalInt(const char* name, int defaultValue)
{
    if (!state || !name) {
        return defaultValue;
    }

    lua_getglobal(state, name);
    if (lua_isnumber(state, -1)) {
        int value = static_cast<int>(lua_tointeger(state, -1));
        lua_pop(state, 1);
        return value;
    }
    lua_pop(state, 1);
    return defaultValue;
}

string LuaContext::getGlobalString(const char* name, const string& defaultValue)
{
    if (!state || !name) {
        return defaultValue;
    }

    lua_getglobal(state, name);
    if (lua_isstring(state, -1)) {
        const char* str = lua_tostring(state, -1);
        string value = str ? str : "";
        lua_pop(state, 1);
        return value;
    }
    lua_pop(state, 1);
    return defaultValue;
}

void LuaContext::setGlobalInt(const char* name, int value)
{
    if (state && name) {
        lua_pushinteger(state, value);
        lua_setglobal(state, name);
    }
}

void LuaContext::setGlobalString(const char* name, const string& value)
{
    if (state && name) {
        lua_pushstring(state, value.c_str());
        lua_setglobal(state, name);
    }
}

}

