#ifndef GAME_G_LUA_H
#define GAME_G_LUA_H

///////////////////////////////////////////////////////////////////////////////
//
// g_lua.h - Lua integration for Jaymod
// Based on ETLegacy's Lua implementation
//
///////////////////////////////////////////////////////////////////////////////

#include <base/lua/public.h>

// Maximum number of Lua VMs that can be loaded
#define LUA_NUM_VM 64

// Maximum size of a Lua file (1MB)
#define LUA_MAX_FSIZE (1024 * 1024)

///////////////////////////////////////////////////////////////////////////////
// Lua VM structure
///////////////////////////////////////////////////////////////////////////////

typedef struct lua_vm_s {
    int           id;
    char          file_name[MAX_QPATH];
    char          mod_name[MAX_CVAR_VALUE_STRING];
    char          mod_signature[41];
    char*         code;
    int           code_size;
    int           err;
    lua_State*    L;
} lua_vm_t;

///////////////////////////////////////////////////////////////////////////////
// Global Lua VM array
///////////////////////////////////////////////////////////////////////////////

extern lua_vm_t* lVM[LUA_NUM_VM];

///////////////////////////////////////////////////////////////////////////////
// Lua API functions
///////////////////////////////////////////////////////////////////////////////

// Initialize Lua subsystem
qboolean G_LuaInit(void);

// Shutdown Lua subsystem
void G_LuaShutdown(void);

// Restart Lua subsystem
void G_LuaRestart(void);

// Print status of loaded Lua modules
void G_LuaStatus(gentity_t* ent);

// Start a Lua VM
qboolean G_LuaStartVM(lua_vm_t* vm);

// Stop a Lua VM
void G_LuaStopVM(lua_vm_t* vm);

// Run an isolated Lua module
qboolean G_LuaRunIsolated(const char* modName);

// Call a Lua function
qboolean G_LuaCall(lua_vm_t* vm, const char* func, int nargs, int nresults);

// Get a named function from a VM
qboolean G_LuaGetNamedFunction(lua_vm_t* vm, const char* name);

// Get the VM for a given lua_State
lua_vm_t* G_LuaGetVM(lua_State* L);

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_G_LUA_H
