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
// Lua Callback Hooks - Called from game code to invoke Lua scripts
///////////////////////////////////////////////////////////////////////////////

// Called when qagame initializes
void G_LuaHook_InitGame(int levelTime, int randomSeed, int restart);

// Called when qagame shuts down
void G_LuaHook_ShutdownGame(int restart);

// Called every server frame
void G_LuaHook_RunFrame(int levelTime);

// Called when a client connects
qboolean G_LuaHook_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot, char* reason);

// Called when a client disconnects
void G_LuaHook_ClientDisconnect(int clientNum);

// Called when a client begins
void G_LuaHook_ClientBegin(int clientNum);

// Called when a client's userinfo changes
void G_LuaHook_ClientUserinfoChanged(int clientNum);

// Called when a client spawns
void G_LuaHook_ClientSpawn(int clientNum, qboolean revived, qboolean teamChange, qboolean restoreHealth);

// Called when a client command is received
qboolean G_LuaHook_ClientCommand(int clientNum, char* command);

// Called when a console command is entered
qboolean G_LuaHook_ConsoleCommand(char* command);

// Called when text is printed
void G_LuaHook_Print(char* text);

// Called when a player dies (obituary)
void G_LuaHook_Obituary(int victim, int killer, int meansOfDeath);

// Called when damage is dealt
qboolean G_LuaHook_Damage(int target, int attacker, int damage, int dflags, int mod);

// Called when a player chats
qboolean G_LuaHook_ChatMessage(int clientNum, int mode, int chatType, char* message);

// Called when a weapon is fired
qboolean G_LuaHook_WeaponFire(int clientNum, int weapon);

// Called when a player is revived
void G_LuaHook_Revive(int clientNum, int reviverNum);

// Called when a player's skill is set
qboolean G_LuaHook_SetPlayerSkill(int clientNum, int skill, int level);

// Called when a player gets a skill upgrade
qboolean G_LuaHook_UpgradeSkill(int clientNum, int skill);

// Called when a fixed MG42 is fired
qboolean G_LuaHook_FixedMGFire(int clientNum);

// Called when a mounted MG42 is fired
qboolean G_LuaHook_MountedMGFire(int clientNum);

// Called when an anti-aircraft gun is fired
qboolean G_LuaHook_AAGunFire(int clientNum);

// Called when entities are spawned from map string
void G_LuaHook_SpawnEntitiesFromString(void);

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_G_LUA_H
