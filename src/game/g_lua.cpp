///////////////////////////////////////////////////////////////////////////////
//
// g_lua.cpp - Lua integration for Jaymod
// Based on ETLegacy's Lua implementation
//
///////////////////////////////////////////////////////////////////////////////

#include <bgame/impl.h>
#include <game/g_lua.h>

///////////////////////////////////////////////////////////////////////////////
// Global Lua VM array
///////////////////////////////////////////////////////////////////////////////

lua_vm_t* lVM[LUA_NUM_VM];

///////////////////////////////////////////////////////////////////////////////
// Lua API library functions for scripts
///////////////////////////////////////////////////////////////////////////////

// et.G_Print(text) - Print text to the server console
static int _et_G_Print(lua_State* L)
{
    char text[1024];
    Q_strncpyz(text, luaL_checkstring(L, 1), sizeof(text));
    G_Printf("%s", text);
    return 0;
}

// et.G_LogPrint(text) - Print text to the server console and log
static int _et_G_LogPrint(lua_State* L)
{
    char text[1024];
    Q_strncpyz(text, luaL_checkstring(L, 1), sizeof(text));
    G_LogPrintf("%s", text);
    return 0;
}

// et.RegisterModname(modname) - Register a name for this module
static int _et_RegisterModname(lua_State* L)
{
    const char* modname = luaL_checkstring(L, 1);
    if (modname) {
        lua_vm_t* vm = G_LuaGetVM(L);
        if (vm) {
            Q_strncpyz(vm->mod_name, modname, sizeof(vm->mod_name));
        }
    }
    return 0;
}

// et.FindSelf() - Returns the assigned Lua VM slot number
static int _et_FindSelf(lua_State* L)
{
    lua_vm_t* vm = G_LuaGetVM(L);
    if (vm) {
        lua_pushinteger(L, vm->id);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// et.trap_Cvar_Get(name) - Get a cvar value
static int _et_trap_Cvar_Get(lua_State* L)
{
    char buff[MAX_CVAR_VALUE_STRING];
    const char* cvarname = luaL_checkstring(L, 1);
    trap_Cvar_VariableStringBuffer(cvarname, buff, sizeof(buff));
    lua_pushstring(L, buff);
    return 1;
}

// et.trap_Cvar_Set(name, value) - Set a cvar value
static int _et_trap_Cvar_Set(lua_State* L)
{
    const char* cvarname = luaL_checkstring(L, 1);
    const char* cvarvalue = luaL_checkstring(L, 2);
    trap_Cvar_Set(cvarname, cvarvalue);
    return 0;
}

// et.trap_SendConsoleCommand(when, command) - Send command to server console
static int _et_trap_SendConsoleCommand(lua_State* L)
{
    int when = (int)luaL_checkinteger(L, 1);
    const char* cmd = luaL_checkstring(L, 2);
    trap_SendConsoleCommand(when, cmd);
    return 0;
}

// et.trap_SendServerCommand(clientnum, command) - Send command to client
static int _et_trap_SendServerCommand(lua_State* L)
{
    int clientnum = (int)luaL_checkinteger(L, 1);
    const char* cmd = luaL_checkstring(L, 2);
    trap_SendServerCommand(clientnum, cmd);
    return 0;
}

// et.trap_Argc() - Returns number of command line arguments
static int _et_trap_Argc(lua_State* L)
{
    lua_pushinteger(L, trap_Argc());
    return 1;
}

// et.trap_Argv(index) - Returns the command line argument at index
static int _et_trap_Argv(lua_State* L)
{
    char buff[MAX_STRING_CHARS];
    int argnum = (int)luaL_checkinteger(L, 1);
    trap_Argv(argnum, buff, sizeof(buff));
    lua_pushstring(L, buff);
    return 1;
}

// et.trap_Milliseconds() - Returns level time in milliseconds
static int _et_trap_Milliseconds(lua_State* L)
{
    lua_pushinteger(L, trap_Milliseconds());
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Lua library registration
///////////////////////////////////////////////////////////////////////////////

static const luaL_Reg etlib[] = {
    { "G_Print",                 _et_G_Print                 },
    { "G_LogPrint",              _et_G_LogPrint              },
    { "RegisterModname",         _et_RegisterModname         },
    { "FindSelf",                _et_FindSelf                },
    { "trap_Cvar_Get",           _et_trap_Cvar_Get           },
    { "trap_Cvar_Set",           _et_trap_Cvar_Set           },
    { "trap_SendConsoleCommand", _et_trap_SendConsoleCommand },
    { "trap_SendServerCommand",  _et_trap_SendServerCommand  },
    { "trap_Argc",               _et_trap_Argc               },
    { "trap_Argv",               _et_trap_Argv               },
    { "trap_Milliseconds",       _et_trap_Milliseconds       },
    { NULL,                      NULL                        }
};

///////////////////////////////////////////////////////////////////////////////
// G_LuaGetVM - Get the VM for a given lua_State
///////////////////////////////////////////////////////////////////////////////

lua_vm_t* G_LuaGetVM(lua_State* L)
{
    for (int i = 0; i < LUA_NUM_VM; i++) {
        if (lVM[i] && lVM[i]->L == L) {
            return lVM[i];
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaCall - Call a Lua function on the stack
///////////////////////////////////////////////////////////////////////////////

qboolean G_LuaCall(lua_vm_t* vm, const char* func, int nargs, int nresults)
{
    switch (lua_pcall(vm->L, nargs, nresults, 0)) {
    case LUA_OK:
        return qtrue;
    case LUA_ERRRUN:
        G_Printf("Lua API: %s error running lua script: '%s'\n", func, lua_tostring(vm->L, -1));
        lua_pop(vm->L, 1);
        vm->err++;
        return qfalse;
    case LUA_ERRMEM:
        G_Printf("Lua API: memory allocation error (%s)\n", vm->file_name);
        vm->err++;
        return qfalse;
    case LUA_ERRERR:
        G_Printf("Lua API: traceback error (%s)\n", vm->file_name);
        vm->err++;
        return qfalse;
    default:
        return qfalse;
    }
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaGetNamedFunction - Get a named function from a VM
///////////////////////////////////////////////////////////////////////////////

qboolean G_LuaGetNamedFunction(lua_vm_t* vm, const char* name)
{
    if (vm->L) {
        lua_getglobal(vm->L, name);
        if (lua_isfunction(vm->L, -1)) {
            return qtrue;
        } else {
            lua_pop(vm->L, 1);
            return qfalse;
        }
    }
    return qfalse;
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaStartVM - Start a Lua VM
///////////////////////////////////////////////////////////////////////////////

qboolean G_LuaStartVM(lua_vm_t* vm)
{
    int res;

    // Open a new lua state
    vm->L = luaL_newstate();
    if (!vm->L) {
        G_Printf("Lua API: failed to initialise Lua state.\n");
        return qfalse;
    }

    // Initialize the lua state with standard libraries
    luaL_openlibs(vm->L);

    // Register et library
    luaL_newlib(vm->L, etlib);
    lua_setglobal(vm->L, "et");

    // Load the Lua code
    res = luaL_loadbuffer(vm->L, vm->code, vm->code_size, vm->file_name);

    switch (res) {
    case LUA_OK:
        break;
    case LUA_ERRSYNTAX:
        G_Printf("Lua API: syntax error during pre-compilation: %s\n", lua_tostring(vm->L, -1));
        lua_pop(vm->L, 1);
        vm->err++;
        return qfalse;
    case LUA_ERRMEM:
        G_Printf("Lua API: memory allocation error #1 (%s)\n", vm->file_name);
        vm->err++;
        return qfalse;
    default:
        G_Printf("Lua API: unknown error %i (%s)\n", res, vm->file_name);
        vm->err++;
        return qfalse;
    }

    // Execute the code
    if (!G_LuaCall(vm, "G_LuaStartVM", 0, 0)) {
        G_Printf("Lua API: Lua VM start failed (%s)\n", vm->file_name);
        return qfalse;
    }

    // Load the code
    G_Printf("Lua API: file '%s' loaded into Lua VM\n", vm->file_name);
    return qtrue;
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaStopVM - Stop a Lua VM
///////////////////////////////////////////////////////////////////////////////

void G_LuaStopVM(lua_vm_t* vm)
{
    if (vm == NULL) {
        return;
    }
    
    if (vm->code != NULL) {
        free(vm->code);
        vm->code = NULL;
    }
    
    if (vm->L) {
        // Call et_Quit if it exists
        if (G_LuaGetNamedFunction(vm, "et_Quit")) {
            G_LuaCall(vm, "et_Quit", 0, 0);
        }
        lua_close(vm->L);
        vm->L = NULL;
    }
    
    if (vm->id >= 0) {
        if (lVM[vm->id] == vm) {
            lVM[vm->id] = NULL;
        }
        if (!vm->err) {
            G_Printf("Lua API: Lua module [%s] [%s] unloaded.\n", vm->file_name, vm->mod_signature);
        }
    }
    
    free(vm);
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaRunIsolated - Run a Lua module in isolation
///////////////////////////////////////////////////////////////////////////////

qboolean G_LuaRunIsolated(const char* modName)
{
    int freeVM, flen = 0;
    char filename[MAX_QPATH];
    char* code;
    fileHandle_t f;
    lua_vm_t* vm;

    // Find a free VM slot
    for (freeVM = 0; freeVM < LUA_NUM_VM; freeVM++) {
        if (lVM[freeVM] == NULL) {
            break;
        }
    }

    if (freeVM == LUA_NUM_VM) {
        G_Printf("Lua API: no free VMs left to load module: \"%s\"\n", modName);
        return qfalse;
    }

    Q_strncpyz(filename, modName, sizeof(filename));

    // Add .lua extension if not present
    if (strlen(filename) < 4 || Q_stricmp(filename + strlen(filename) - 4, ".lua") != 0) {
        Q_strcat(filename, sizeof(filename), ".lua");
    }

    // Try to open lua file
    flen = trap_FS_FOpenFile(filename, &f, FS_READ);
    if (flen < 0) {
        G_Printf("Lua API: can not open file '%s'\n", filename);
        return qfalse;
    } else if (flen > LUA_MAX_FSIZE) {
        G_Printf("Lua API: ignoring file '%s' (too big)\n", filename);
        trap_FS_FCloseFile(f);
        return qfalse;
    }

    code = (char*)malloc(flen + 1);
    if (code == NULL) {
        G_Printf("Lua API: memory allocation error for '%s' data\n", filename);
        trap_FS_FCloseFile(f);
        return qfalse;
    }

    trap_FS_Read(code, flen, f);
    *(code + flen) = '\0';
    trap_FS_FCloseFile(f);

    // Compute signature hash for the code
    // Using a simple hash algorithm since SHA1 is not available
    {
        unsigned long hash = 5381;
        int c;
        char* p = code;
        while ((c = *p++)) {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        
        // Also incorporate file length for uniqueness
        hash ^= (unsigned long)flen;
        
        // Init lua_vm_t struct
        vm = (lua_vm_t*)malloc(sizeof(lua_vm_t));
        if (vm == NULL) {
            G_Printf("Lua API: vm memory allocation error for %s data\n", filename);
            free(code);
            return qfalse;
        }

        vm->id = -1;
        Q_strncpyz(vm->file_name, filename, sizeof(vm->file_name));
        Q_strncpyz(vm->mod_name, "", sizeof(vm->mod_name));
        // Store hash as hex signature
        Com_sprintf(vm->mod_signature, sizeof(vm->mod_signature), "%08lX%08lX", 
            (hash >> 32) & 0xFFFFFFFF, hash & 0xFFFFFFFF);
    }
    vm->code = code;
    vm->code_size = flen;
    vm->err = 0;
    vm->L = NULL;

    // Start lua virtual machine
    if (G_LuaStartVM(vm)) {
        vm->id = freeVM;
        lVM[freeVM] = vm;
        return qtrue;
    } else {
        G_LuaStopVM(vm);
        return qfalse;
    }
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaInit - Initialize Lua subsystem
///////////////////////////////////////////////////////////////////////////////

qboolean G_LuaInit(void)
{
    int i, num_vm = 0;
    char buff[MAX_CVAR_VALUE_STRING];
    char* crt;

    // Initialize VM array
    for (i = 0; i < LUA_NUM_VM; i++) {
        lVM[i] = NULL;
    }

    // Get lua_modules cvar
    trap_Cvar_VariableStringBuffer("lua_modules", buff, sizeof(buff));

    if (buff[0] == '\0') {
        G_Printf("Lua API: no Lua files set (use lua_modules cvar)\n");
        return qtrue;
    }

    // Parse lua_modules cvar and load each module
    int len = strlen(buff);
    crt = buff;
    for (i = 0; i <= len; i++) {
        if (buff[i] == ' ' || buff[i] == '\0' || buff[i] == ',' || buff[i] == ';') {
            buff[i] = '\0';

            if (num_vm >= LUA_NUM_VM) {
                G_Printf("Lua API: too many lua files specified, only the first %d have been loaded\n", LUA_NUM_VM);
                break;
            }

            if (crt[0] != '\0') {
                if (G_LuaRunIsolated(crt)) {
                    num_vm++;
                }
            }

            // Prepare for next iteration
            if (i + 1 < len) {
                crt = buff + i + 1;
            } else {
                crt = NULL;
            }
        }
    }

    return qtrue;
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaShutdown - Shutdown Lua subsystem
///////////////////////////////////////////////////////////////////////////////

void G_LuaShutdown(void)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            G_LuaStopVM(vm);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaRestart - Restart Lua subsystem
///////////////////////////////////////////////////////////////////////////////

void G_LuaRestart(void)
{
    G_LuaShutdown();
    G_LuaInit();
}

///////////////////////////////////////////////////////////////////////////////
// G_LuaStatus - Print status of loaded Lua modules
///////////////////////////////////////////////////////////////////////////////

void G_LuaStatus(gentity_t* ent)
{
    int i, cnt = 0;

    // Count loaded VMs
    for (i = 0; i < LUA_NUM_VM; i++) {
        if (lVM[i]) {
            cnt++;
        }
    }

    if (cnt == 0) {
        if (ent) {
            trap_SendServerCommand(ent - g_entities, "print \"Lua API: no scripts loaded.\n\"");
        } else {
            G_Printf("Lua API: no scripts loaded.\n");
        }
        return;
    }

    // Print header
    if (ent) {
        trap_SendServerCommand(ent - g_entities, va("print \"Lua API: showing lua information (%d module%s loaded)\n\"", cnt, cnt == 1 ? "" : "s"));
        trap_SendServerCommand(ent - g_entities, "print \"VM Modname                  Signature                                Filename\n\"");
        trap_SendServerCommand(ent - g_entities, "print \"-- ------------------------ ---------------------------------------- ------------------------\n\"");
    } else {
        G_Printf("Lua API: showing lua information (%d module%s loaded)\n", cnt, cnt == 1 ? "" : "s");
        G_Printf("VM Modname                  Signature                                Filename\n");
        G_Printf("-- ------------------------ ---------------------------------------- ------------------------\n");
    }

    // Print each loaded VM
    for (i = 0; i < LUA_NUM_VM; i++) {
        if (lVM[i]) {
            if (ent) {
                trap_SendServerCommand(ent - g_entities, va("print \"%2d %-24s %-40s %-24s\n\"", 
                    lVM[i]->id, 
                    lVM[i]->mod_name[0] ? lVM[i]->mod_name : "(unnamed)", 
                    lVM[i]->mod_signature, 
                    lVM[i]->file_name));
            } else {
                G_Printf("%2d %-24s %-40s %-24s\n", 
                    lVM[i]->id, 
                    lVM[i]->mod_name[0] ? lVM[i]->mod_name : "(unnamed)", 
                    lVM[i]->mod_signature, 
                    lVM[i]->file_name);
            }
        }
    }

    // Print footer
    if (ent) {
        trap_SendServerCommand(ent - g_entities, "print \"-- ------------------------ ---------------------------------------- ------------------------\n\"");
    } else {
        G_Printf("-- ------------------------ ---------------------------------------- ------------------------\n");
    }
}

///////////////////////////////////////////////////////////////////////////////
// Lua Callback Hooks - Called from game code to invoke Lua scripts
///////////////////////////////////////////////////////////////////////////////

// G_LuaHook_InitGame - Called when qagame initializes
void G_LuaHook_InitGame(int levelTime, int randomSeed, int restart)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_InitGame")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, levelTime);
            lua_pushinteger(vm->L, randomSeed);
            lua_pushinteger(vm->L, restart);
            // Call
            if (!G_LuaCall(vm, "et_InitGame", 3, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_ShutdownGame - Called when qagame shuts down
void G_LuaHook_ShutdownGame(int restart)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ShutdownGame")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, restart);
            // Call
            if (!G_LuaCall(vm, "et_ShutdownGame", 1, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_RunFrame - Called every server frame
void G_LuaHook_RunFrame(int levelTime)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_RunFrame")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, levelTime);
            // Call
            if (!G_LuaCall(vm, "et_RunFrame", 1, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_ClientConnect - Called when a client connects
qboolean G_LuaHook_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot, char* reason)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ClientConnect")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            lua_pushinteger(vm->L, (int)firstTime);
            lua_pushinteger(vm->L, (int)isBot);
            // Call
            if (!G_LuaCall(vm, "et_ClientConnect", 3, 1)) {
                continue;
            }
            // Return values
            if (lua_isstring(vm->L, -1)) {
                Q_strncpyz(reason, lua_tostring(vm->L, -1), MAX_STRING_CHARS);
                lua_pop(vm->L, 1);
                return qtrue;
            }
            lua_pop(vm->L, 1);
        }
    }
    return qfalse;
}

// G_LuaHook_ClientDisconnect - Called when a client disconnects
void G_LuaHook_ClientDisconnect(int clientNum)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ClientDisconnect")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            // Call
            if (!G_LuaCall(vm, "et_ClientDisconnect", 1, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_ClientBegin - Called when a client begins
void G_LuaHook_ClientBegin(int clientNum)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ClientBegin")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            // Call
            if (!G_LuaCall(vm, "et_ClientBegin", 1, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_ClientUserinfoChanged - Called when a client's userinfo changes
void G_LuaHook_ClientUserinfoChanged(int clientNum)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ClientUserinfoChanged")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            // Call
            if (!G_LuaCall(vm, "et_ClientUserinfoChanged", 1, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_ClientSpawn - Called when a client spawns
void G_LuaHook_ClientSpawn(int clientNum, qboolean revived, qboolean teamChange, qboolean restoreHealth)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ClientSpawn")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            lua_pushinteger(vm->L, (int)revived);
            lua_pushinteger(vm->L, (int)teamChange);
            lua_pushinteger(vm->L, (int)restoreHealth);
            // Call
            if (!G_LuaCall(vm, "et_ClientSpawn", 4, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_ClientCommand - Called when a client command is received
qboolean G_LuaHook_ClientCommand(int clientNum, char* command)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ClientCommand")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            lua_pushstring(vm->L, command);
            // Call
            if (!G_LuaCall(vm, "et_ClientCommand", 2, 1)) {
                continue;
            }
            // Return values
            if (lua_isnumber(vm->L, -1)) {
                if (lua_tointeger(vm->L, -1) == 1) {
                    lua_pop(vm->L, 1);
                    return qtrue;
                }
            }
            lua_pop(vm->L, 1);
        }
    }
    return qfalse;
}

// G_LuaHook_ConsoleCommand - Called when a console command is entered
qboolean G_LuaHook_ConsoleCommand(char* command)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ConsoleCommand")) {
                continue;
            }
            // Arguments
            lua_pushstring(vm->L, command);
            // Call
            if (!G_LuaCall(vm, "et_ConsoleCommand", 1, 1)) {
                continue;
            }
            // Return values
            if (lua_isnumber(vm->L, -1)) {
                if (lua_tointeger(vm->L, -1) == 1) {
                    lua_pop(vm->L, 1);
                    return qtrue;
                }
            }
            lua_pop(vm->L, 1);
        }
    }
    return qfalse;
}

// G_LuaHook_Print - Called when text is printed
void G_LuaHook_Print(char* text)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_Print")) {
                continue;
            }
            // Arguments
            lua_pushstring(vm->L, text);
            // Call
            if (!G_LuaCall(vm, "et_Print", 1, 0)) {
                continue;
            }
        }
    }
}
