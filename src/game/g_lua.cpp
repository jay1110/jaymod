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

// et.ConcatArgs(index) - Returns all arguments starting from index concatenated
static int _et_ConcatArgs(lua_State* L)
{
    int index = (int)luaL_checkinteger(L, 1);
    lua_pushstring(L, ConcatArgs(index));
    return 1;
}

// et.trap_GetConfigstring(index) - Get a configstring value
static int _et_trap_GetConfigstring(lua_State* L)
{
    char buff[MAX_STRING_CHARS];
    int index = (int)luaL_checkinteger(L, 1);
    trap_GetConfigstring(index, buff, sizeof(buff));
    lua_pushstring(L, buff);
    return 1;
}

// et.trap_SetConfigstring(index, value) - Set a configstring value
static int _et_trap_SetConfigstring(lua_State* L)
{
    int index = (int)luaL_checkinteger(L, 1);
    const char* csv = luaL_checkstring(L, 2);
    trap_SetConfigstring(index, csv);
    return 0;
}

// et.trap_GetUserinfo(clientnum) - Get a client's userinfo string
static int _et_trap_GetUserinfo(lua_State* L)
{
    char buff[MAX_STRING_CHARS];
    int clientnum = (int)luaL_checkinteger(L, 1);
    trap_GetUserinfo(clientnum, buff, sizeof(buff));
    lua_pushstring(L, buff);
    return 1;
}

// et.trap_SetUserinfo(clientnum, userinfo) - Set a client's userinfo string
static int _et_trap_SetUserinfo(lua_State* L)
{
    int clientnum = (int)luaL_checkinteger(L, 1);
    const char* userinfo = luaL_checkstring(L, 2);
    trap_SetUserinfo(clientnum, userinfo);
    return 0;
}

// et.trap_DropClient(clientnum, reason, bantime) - Disconnect a client
static int _et_trap_DropClient(lua_State* L)
{
    int clientnum = (int)luaL_checkinteger(L, 1);
    const char* reason = luaL_checkstring(L, 2);
    int ban_time = (int)luaL_checkinteger(L, 3);
    trap_DropClient(clientnum, reason, ban_time);
    return 0;
}

// et.ClientUserinfoChanged(clientnum) - Notify that userinfo changed
static int _et_ClientUserinfoChanged(lua_State* L)
{
    int clientnum = (int)luaL_checkinteger(L, 1);
    ClientUserinfoChanged(clientnum);
    return 0;
}

// et.ClientNumberFromString(string) - Find client number from partial name match
static int _et_ClientNumberFromString(lua_State* L)
{
    const char* search = luaL_checkstring(L, 1);
    int pids[MAX_CLIENTS];
    
    // Only send exact matches, otherwise nil
    if (ClientNumbersFromString((char*)search, pids) == 1) {
        lua_pushinteger(L, pids[0]);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// et.FindMod(vmnumber) - Returns the name and signature for a VM slot
static int _et_FindMod(lua_State* L)
{
    int vmnumber = (int)luaL_checkinteger(L, 1);
    
    if (vmnumber >= 0 && vmnumber < LUA_NUM_VM && lVM[vmnumber]) {
        lua_vm_t* vm = lVM[vmnumber];
        lua_pushstring(L, vm->mod_name);
        lua_pushstring(L, vm->mod_signature);
    } else {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}

// et.IPCSend(vmnumber, message) - Send a message to another VM
static int _et_IPCSend(lua_State* L)
{
    int vmnumber = (int)luaL_checkinteger(L, 1);
    const char* message = luaL_checkstring(L, 2);
    
    lua_vm_t* sender = G_LuaGetVM(L);
    lua_vm_t* vm = NULL;
    
    if (vmnumber >= 0 && vmnumber < LUA_NUM_VM) {
        vm = lVM[vmnumber];
    }
    
    if (!vm || vm->err) {
        lua_pushinteger(L, 0);
        return 1;
    }
    
    // Find callback
    if (!G_LuaGetNamedFunction(vm, "et_IPCReceive")) {
        lua_pushinteger(L, 0);
        return 1;
    }
    
    // Arguments
    if (sender) {
        lua_pushinteger(vm->L, sender->id);
    } else {
        lua_pushnil(vm->L);
    }
    lua_pushstring(vm->L, message);
    
    // Call
    if (!G_LuaCall(vm, "et.IPCSend", 2, 0)) {
        lua_pushinteger(L, 0);
        return 1;
    }
    
    // Success
    lua_pushinteger(L, 1);
    return 1;
}

// et.Info_RemoveKey(infostring, key) - Remove a key from an infostring
static int _et_Info_RemoveKey(lua_State* L)
{
    char buff[MAX_INFO_STRING];
    const char* key = luaL_checkstring(L, 2);
    Q_strncpyz(buff, luaL_checkstring(L, 1), sizeof(buff));
    Info_RemoveKey(buff, key);
    lua_pushstring(L, buff);
    return 1;
}

// et.Info_SetValueForKey(infostring, key, value) - Set a value in an infostring
static int _et_Info_SetValueForKey(lua_State* L)
{
    char buff[MAX_INFO_STRING];
    const char* key = luaL_checkstring(L, 2);
    const char* value = luaL_checkstring(L, 3);
    Q_strncpyz(buff, luaL_checkstring(L, 1), sizeof(buff));
    Info_SetValueForKey(buff, key, value);
    lua_pushstring(L, buff);
    return 1;
}

// et.Info_ValueForKey(infostring, key) - Get a value from an infostring
static int _et_Info_ValueForKey(lua_State* L)
{
    const char* infostring = luaL_checkstring(L, 1);
    const char* key = luaL_checkstring(L, 2);
    lua_pushstring(L, Info_ValueForKey(infostring, key));
    return 1;
}

// et.Q_CleanStr(string) - Remove color codes from a string
static int _et_Q_CleanStr(lua_State* L)
{
    char buff[MAX_STRING_CHARS];
    Q_strncpyz(buff, luaL_checkstring(L, 1), sizeof(buff));
    Q_CleanStr(buff);
    lua_pushstring(L, buff);
    return 1;
}

// et.G_Say(clientNum, mode, text) - Make a client say something
static int _et_G_Say(lua_State* L)
{
    int clientnum = (int)luaL_checkinteger(L, 1);
    int mode = (int)luaL_checkinteger(L, 2);
    const char* text = luaL_checkstring(L, 3);
    G_Say(g_entities + clientnum, NULL, mode, text);
    return 0;
}

// et.isBitSet(bit, value) - Check if a bit is set in a value
static int _et_isBitSet(lua_State* L)
{
    int b = (int)luaL_checkinteger(L, 1);
    int v = (int)luaL_checkinteger(L, 2);
    lua_pushboolean(L, (v & b) ? 1 : 0);
    return 1;
}

// et.G_Damage(target, inflictor, attacker, damage, dflags, mod) - Damage an entity
static int _et_G_Damage(lua_State* L)
{
    int target = (int)luaL_checkinteger(L, 1);
    int inflictor = (int)luaL_checkinteger(L, 2);
    int attacker = (int)luaL_checkinteger(L, 3);
    int damage = (int)luaL_checkinteger(L, 4);
    int dflags = (int)luaL_checkinteger(L, 5);
    int mod = (int)luaL_checkinteger(L, 6);
    
    G_Damage(g_entities + target,
             g_entities + inflictor,
             g_entities + attacker,
             NULL,
             NULL,
             damage,
             dflags,
             (meansOfDeath_t)mod);
    
    return 0;
}

// et.G_SoundIndex(filename) - Get a sound index
static int _et_G_SoundIndex(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    lua_pushinteger(L, G_SoundIndex(filename));
    return 1;
}

// et.G_ModelIndex(filename) - Get a model index
static int _et_G_ModelIndex(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    lua_pushinteger(L, G_ModelIndex((char*)filename));
    return 1;
}

// et.G_globalSound(sound) - Play a global sound
static int _et_G_globalSound(lua_State* L)
{
    const char* sound = luaL_checkstring(L, 1);
    G_globalSound((char*)sound);
    return 0;
}

// et.G_Sound(entnum, soundindex) - Play a sound at an entity's position
static int _et_G_Sound(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    int soundindex = (int)luaL_checkinteger(L, 2);
    G_Sound(g_entities + entnum, soundindex);
    return 0;
}

// Filesystem functions

// et.trap_FS_FOpenFile(filename, mode) - Open a file
static int _et_trap_FS_FOpenFile(lua_State* L)
{
    fileHandle_t fd;
    int len;
    const char* filename = luaL_checkstring(L, 1);
    int mode = (int)luaL_checkinteger(L, 2);
    
    len = trap_FS_FOpenFile(filename, &fd, (fsMode_t)mode);
    lua_pushinteger(L, fd);
    lua_pushinteger(L, len);
    return 2;
}

// et.trap_FS_Read(fd, count) - Read from a file
static int _et_trap_FS_Read(lua_State* L)
{
    fileHandle_t fd = (int)luaL_checkinteger(L, 1);
    int count = (int)luaL_checkinteger(L, 2);
    char* filedata;
    
    filedata = (char*)malloc(count + 1);
    if (filedata == NULL) {
        G_Printf("Lua API: memory allocation error for trap_FS_Read\n");
        lua_pushstring(L, "");
        return 1;
    }
    
    trap_FS_Read(filedata, count, fd);
    *(filedata + count) = '\0';
    lua_pushstring(L, filedata);
    free(filedata);
    return 1;
}

// et.trap_FS_Write(filedata, count, fd) - Write to a file
static int _et_trap_FS_Write(lua_State* L)
{
    const char* filedata = luaL_checkstring(L, 1);
    int count = (int)luaL_checkinteger(L, 2);
    fileHandle_t fd = (int)luaL_checkinteger(L, 3);
    lua_pushinteger(L, trap_FS_Write(filedata, count, fd));
    return 1;
}

// et.trap_FS_FCloseFile(fd) - Close a file
static int _et_trap_FS_FCloseFile(lua_State* L)
{
    fileHandle_t fd = (int)luaL_checkinteger(L, 1);
    trap_FS_FCloseFile(fd);
    return 0;
}

// et.trap_FS_Rename(oldname, newname) - Rename a file
static int _et_trap_FS_Rename(lua_State* L)
{
    const char* oldname = luaL_checkstring(L, 1);
    const char* newname = luaL_checkstring(L, 2);
    trap_FS_Rename(oldname, newname);
    return 0;
}

// Entity functions

// et.G_TempEntity(origin, event) - Spawn a temporary entity
static int _et_G_TempEntity(lua_State* L)
{
    vec3_t origin;
    int event = (int)luaL_checkinteger(L, 2);
    
    // Get origin from table
    lua_pushvalue(L, 1);
    lua_pushnumber(L, 1);
    lua_gettable(L, -2);
    origin[0] = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushnumber(L, 2);
    lua_gettable(L, -2);
    origin[1] = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushnumber(L, 3);
    lua_gettable(L, -2);
    origin[2] = (float)lua_tonumber(L, -1);
    lua_pop(L, 2);
    
    lua_pushinteger(L, G_TempEntity(origin, event) - g_entities);
    return 1;
}

// et.G_FreeEntity(entnum) - Free an entity
static int _et_G_FreeEntity(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    G_FreeEntity(g_entities + entnum);
    return 0;
}

// et.G_EntitiesFree() - Count free entities
static int _et_G_EntitiesFree(lua_State* L)
{
    int count = 0;
    int i;
    for (i = MAX_CLIENTS; i < level.num_entities; i++) {
        if (!g_entities[i].inuse) {
            count++;
        }
    }
    lua_pushinteger(L, count);
    return 1;
}

// et.trap_LinkEntity(entnum) - Link an entity
static int _et_trap_LinkEntity(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    trap_LinkEntity(g_entities + entnum);
    return 0;
}

// et.trap_UnlinkEntity(entnum) - Unlink an entity
static int _et_trap_UnlinkEntity(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    trap_UnlinkEntity(g_entities + entnum);
    return 0;
}

// et.G_Spawn() - Spawn a new entity
static int _et_G_Spawn(lua_State* L)
{
    gentity_t* ent = G_Spawn();
    if (ent) {
        lua_pushinteger(L, ent - g_entities);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// et.G_AddEvent(entnum, event, eventparm) - Add an event to an entity
static int _et_G_AddEvent(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    int event = (int)luaL_checkinteger(L, 2);
    int eventparm = (int)luaL_checkinteger(L, 3);
    G_AddEvent(g_entities + entnum, event, eventparm);
    return 0;
}

// Helper function to get a vec3 from a Lua table
static void _et_getvec3(lua_State* L, int idx, vec3_t vec)
{
    lua_pushnumber(L, 1);
    lua_gettable(L, idx);
    vec[0] = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushnumber(L, 2);
    lua_gettable(L, idx);
    vec[1] = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushnumber(L, 3);
    lua_gettable(L, idx);
    vec[2] = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);
}

// Helper function to push a vec3 to Lua as a table
static void _et_pushvec3(lua_State* L, vec3_t vec)
{
    lua_newtable(L);
    lua_pushnumber(L, vec[0]);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, vec[1]);
    lua_rawseti(L, -2, 2);
    lua_pushnumber(L, vec[2]);
    lua_rawseti(L, -2, 3);
}

// et.gentity_get(entnum, fieldname) - Get a field from an entity
static int _et_gentity_get(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    const char* fieldname = luaL_checkstring(L, 2);
    gentity_t* ent;
    
    if (entnum < 0 || entnum >= MAX_GENTITIES) {
        lua_pushnil(L);
        return 1;
    }
    
    ent = &g_entities[entnum];
    
    if (!ent->inuse) {
        lua_pushnil(L);
        return 1;
    }
    
    // Handle entity fields
    if (!Q_stricmp(fieldname, "classname")) {
        lua_pushstring(L, ent->classname ? ent->classname : "");
    } else if (!Q_stricmp(fieldname, "targetname")) {
        lua_pushstring(L, ent->targetname ? ent->targetname : "");
    } else if (!Q_stricmp(fieldname, "target")) {
        lua_pushstring(L, ent->target ? ent->target : "");
    } else if (!Q_stricmp(fieldname, "health")) {
        lua_pushinteger(L, ent->health);
    } else if (!Q_stricmp(fieldname, "damage")) {
        lua_pushinteger(L, ent->damage);
    } else if (!Q_stricmp(fieldname, "spawnflags")) {
        lua_pushinteger(L, ent->spawnflags);
    } else if (!Q_stricmp(fieldname, "clipmask")) {
        lua_pushinteger(L, ent->clipmask);
    } else if (!Q_stricmp(fieldname, "count")) {
        lua_pushinteger(L, ent->count);
    } else if (!Q_stricmp(fieldname, "flags")) {
        lua_pushinteger(L, ent->flags);
    } else if (!Q_stricmp(fieldname, "inuse")) {
        lua_pushinteger(L, ent->inuse);
    } else if (!Q_stricmp(fieldname, "origin")) {
        _et_pushvec3(L, ent->r.currentOrigin);
    } else if (!Q_stricmp(fieldname, "angles")) {
        _et_pushvec3(L, ent->r.currentAngles);
    } else if (!Q_stricmp(fieldname, "mins")) {
        _et_pushvec3(L, ent->r.mins);
    } else if (!Q_stricmp(fieldname, "maxs")) {
        _et_pushvec3(L, ent->r.maxs);
    } else if (!Q_stricmp(fieldname, "s.eType")) {
        lua_pushinteger(L, ent->s.eType);
    } else if (!Q_stricmp(fieldname, "s.eFlags")) {
        lua_pushinteger(L, ent->s.eFlags);
    } else if (!Q_stricmp(fieldname, "s.number")) {
        lua_pushinteger(L, ent->s.number);
    } else if (!Q_stricmp(fieldname, "s.clientNum")) {
        lua_pushinteger(L, ent->s.clientNum);
    } else if (!Q_stricmp(fieldname, "s.weapon")) {
        lua_pushinteger(L, ent->s.weapon);
    } else if (!Q_stricmp(fieldname, "s.teamNum")) {
        lua_pushinteger(L, ent->s.teamNum);
    } else if (!Q_stricmp(fieldname, "s.modelindex")) {
        lua_pushinteger(L, ent->s.modelindex);
    } else if (!Q_stricmp(fieldname, "s.modelindex2")) {
        lua_pushinteger(L, ent->s.modelindex2);
    } else if (ent->client) {
        // Client fields
        gclient_t* client = ent->client;
        if (!Q_stricmp(fieldname, "pers.netname")) {
            lua_pushstring(L, client->pers.netname);
        } else if (!Q_stricmp(fieldname, "sess.sessionTeam")) {
            lua_pushinteger(L, client->sess.sessionTeam);
        } else if (!Q_stricmp(fieldname, "sess.playerType")) {
            lua_pushinteger(L, client->sess.playerType);
        } else if (!Q_stricmp(fieldname, "sess.playerWeapon")) {
            lua_pushinteger(L, client->sess.playerWeapon);
        } else if (!Q_stricmp(fieldname, "ps.stats")) {
            // Return as table
            int i, arrayindex = (int)luaL_optinteger(L, 3, 0);
            if (arrayindex >= 0 && arrayindex < MAX_STATS) {
                lua_pushinteger(L, client->ps.stats[arrayindex]);
            } else {
                lua_pushnil(L);
            }
        } else if (!Q_stricmp(fieldname, "ps.persistant")) {
            int arrayindex = (int)luaL_optinteger(L, 3, 0);
            if (arrayindex >= 0 && arrayindex < MAX_PERSISTANT) {
                lua_pushinteger(L, client->ps.persistant[arrayindex]);
            } else {
                lua_pushnil(L);
            }
        } else if (!Q_stricmp(fieldname, "ps.weapon")) {
            lua_pushinteger(L, client->ps.weapon);
        } else if (!Q_stricmp(fieldname, "ps.pm_type")) {
            lua_pushinteger(L, client->ps.pm_type);
        } else if (!Q_stricmp(fieldname, "ps.pm_flags")) {
            lua_pushinteger(L, client->ps.pm_flags);
        } else if (!Q_stricmp(fieldname, "ps.origin")) {
            _et_pushvec3(L, client->ps.origin);
        } else if (!Q_stricmp(fieldname, "ps.velocity")) {
            _et_pushvec3(L, client->ps.velocity);
        } else if (!Q_stricmp(fieldname, "ps.viewangles")) {
            _et_pushvec3(L, client->ps.viewangles);
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }
    
    return 1;
}

// et.gentity_set(entnum, fieldname, value) - Set a field on an entity
static int _et_gentity_set(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    const char* fieldname = luaL_checkstring(L, 2);
    gentity_t* ent;
    
    if (entnum < 0 || entnum >= MAX_GENTITIES) {
        return 0;
    }
    
    ent = &g_entities[entnum];
    
    if (!ent->inuse) {
        return 0;
    }
    
    // Handle entity fields
    if (!Q_stricmp(fieldname, "health")) {
        ent->health = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "damage")) {
        ent->damage = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "spawnflags")) {
        ent->spawnflags = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "clipmask")) {
        ent->clipmask = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "count")) {
        ent->count = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "flags")) {
        ent->flags = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "origin")) {
        _et_getvec3(L, 3, ent->r.currentOrigin);
    } else if (!Q_stricmp(fieldname, "angles")) {
        _et_getvec3(L, 3, ent->r.currentAngles);
    } else if (!Q_stricmp(fieldname, "mins")) {
        _et_getvec3(L, 3, ent->r.mins);
    } else if (!Q_stricmp(fieldname, "maxs")) {
        _et_getvec3(L, 3, ent->r.maxs);
    } else if (!Q_stricmp(fieldname, "s.eType")) {
        ent->s.eType = (entityType_t)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "s.eFlags")) {
        ent->s.eFlags = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "s.weapon")) {
        ent->s.weapon = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "s.teamNum")) {
        ent->s.teamNum = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "s.modelindex")) {
        ent->s.modelindex = (int)luaL_checkinteger(L, 3);
    } else if (!Q_stricmp(fieldname, "s.modelindex2")) {
        ent->s.modelindex2 = (int)luaL_checkinteger(L, 3);
    } else if (ent->client) {
        // Client fields
        gclient_t* client = ent->client;
        if (!Q_stricmp(fieldname, "ps.stats")) {
            int arrayindex = (int)luaL_checkinteger(L, 3);
            int value = (int)luaL_checkinteger(L, 4);
            if (arrayindex >= 0 && arrayindex < MAX_STATS) {
                client->ps.stats[arrayindex] = value;
            }
        } else if (!Q_stricmp(fieldname, "ps.persistant")) {
            int arrayindex = (int)luaL_checkinteger(L, 3);
            int value = (int)luaL_checkinteger(L, 4);
            if (arrayindex >= 0 && arrayindex < MAX_PERSISTANT) {
                client->ps.persistant[arrayindex] = value;
            }
        } else if (!Q_stricmp(fieldname, "ps.weapon")) {
            client->ps.weapon = (int)luaL_checkinteger(L, 3);
        } else if (!Q_stricmp(fieldname, "ps.pm_type")) {
            client->ps.pm_type = (int)luaL_checkinteger(L, 3);
        } else if (!Q_stricmp(fieldname, "ps.pm_flags")) {
            client->ps.pm_flags = (int)luaL_checkinteger(L, 3);
        } else if (!Q_stricmp(fieldname, "ps.origin")) {
            _et_getvec3(L, 3, client->ps.origin);
        } else if (!Q_stricmp(fieldname, "ps.velocity")) {
            _et_getvec3(L, 3, client->ps.velocity);
        } else if (!Q_stricmp(fieldname, "ps.viewangles")) {
            _et_getvec3(L, 3, client->ps.viewangles);
        }
    }
    
    return 0;
}

// et.G_SetOrigin(entnum, origin) - Set entity origin
static int _et_G_SetOrigin(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    vec3_t origin;
    
    if (entnum < 0 || entnum >= MAX_GENTITIES) {
        return 0;
    }
    
    _et_getvec3(L, 2, origin);
    G_SetOrigin(g_entities + entnum, origin);
    return 0;
}

// et.G_SetAngles(entnum, angles) - Set entity angles
static int _et_G_SetAngles(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    vec3_t angles;
    
    if (entnum < 0 || entnum >= MAX_GENTITIES) {
        return 0;
    }
    
    _et_getvec3(L, 2, angles);
    VectorCopy(angles, g_entities[entnum].r.currentAngles);
    VectorCopy(angles, g_entities[entnum].s.angles);
    return 0;
}

// et.G_GetOrigin(entnum) - Get entity origin
static int _et_G_GetOrigin(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    gentity_t* ent;
    
    if (entnum < 0 || entnum >= MAX_GENTITIES) {
        lua_pushnil(L);
        return 1;
    }
    
    ent = &g_entities[entnum];
    _et_pushvec3(L, ent->r.currentOrigin);
    return 1;
}

// et.G_GetAngles(entnum) - Get entity angles
static int _et_G_GetAngles(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    gentity_t* ent;
    
    if (entnum < 0 || entnum >= MAX_GENTITIES) {
        lua_pushnil(L);
        return 1;
    }
    
    ent = &g_entities[entnum];
    _et_pushvec3(L, ent->r.currentAngles);
    return 1;
}

// et.G_AddSkillPoints(entnum, skill, points) - Add skill points to a player
static int _et_G_AddSkillPoints(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    int skill = (int)luaL_checkinteger(L, 2);
    float points = (float)luaL_checknumber(L, 3);
    gentity_t* ent;
    
    if (entnum < 0 || entnum >= MAX_CLIENTS) {
        return 0;
    }
    
    ent = &g_entities[entnum];
    if (ent->client) {
        G_AddSkillPoints(ent, (skillType_t)skill, points);
    }
    return 0;
}

// et.G_LoseSkillPoints(entnum, skill, points) - Remove skill points from a player
static int _et_G_LoseSkillPoints(lua_State* L)
{
    int entnum = (int)luaL_checkinteger(L, 1);
    int skill = (int)luaL_checkinteger(L, 2);
    float points = (float)luaL_checknumber(L, 3);
    gentity_t* ent;
    
    if (entnum < 0 || entnum >= MAX_CLIENTS) {
        return 0;
    }
    
    ent = &g_entities[entnum];
    if (ent->client) {
        G_LoseSkillPoints(ent, (skillType_t)skill, points);
    }
    return 0;
}

// et.trap_Trace(start, mins, maxs, end, entNum, mask) - Perform a trace
static int _et_trap_Trace(lua_State* L)
{
    trace_t tr;
    vec3_t start, end, mins, maxs;
    int entNum, mask;
    
    _et_getvec3(L, 1, start);
    
    if (!lua_isnil(L, 2)) {
        _et_getvec3(L, 2, mins);
    } else {
        VectorClear(mins);
    }
    
    if (!lua_isnil(L, 3)) {
        _et_getvec3(L, 3, maxs);
    } else {
        VectorClear(maxs);
    }
    
    _et_getvec3(L, 4, end);
    entNum = (int)luaL_checkinteger(L, 5);
    mask = (int)luaL_checkinteger(L, 6);
    
    trap_Trace(&tr, start, mins, maxs, end, entNum, mask);
    
    // Return trace results as table
    lua_newtable(L);
    
    lua_pushstring(L, "allsolid");
    lua_pushboolean(L, tr.allsolid);
    lua_settable(L, -3);
    
    lua_pushstring(L, "startsolid");
    lua_pushboolean(L, tr.startsolid);
    lua_settable(L, -3);
    
    lua_pushstring(L, "fraction");
    lua_pushnumber(L, tr.fraction);
    lua_settable(L, -3);
    
    lua_pushstring(L, "endpos");
    _et_pushvec3(L, tr.endpos);
    lua_settable(L, -3);
    
    lua_pushstring(L, "surfaceFlags");
    lua_pushinteger(L, tr.surfaceFlags);
    lua_settable(L, -3);
    
    lua_pushstring(L, "contents");
    lua_pushinteger(L, tr.contents);
    lua_settable(L, -3);
    
    lua_pushstring(L, "entityNum");
    lua_pushinteger(L, tr.entityNum);
    lua_settable(L, -3);
    
    return 1;
}

// et.trap_FS_GetFileList(path, extension) - Get list of files
static int _et_trap_FS_GetFileList(lua_State* L)
{
    char buff[16384];
    char* s;
    int numfiles, i;
    const char* path = luaL_checkstring(L, 1);
    const char* ext = luaL_checkstring(L, 2);
    
    numfiles = trap_FS_GetFileList(path, ext, buff, sizeof(buff));
    
    lua_newtable(L);
    s = buff;
    for (i = 0; i < numfiles; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushstring(L, s);
        lua_settable(L, -3);
        s += strlen(s) + 1;
    }
    
    return 1;
}

// et.G_XP_Set(clientNum, xp, skill, add) - Set or add XP for a player
static int _et_G_XP_Set(lua_State* L)
{
    int clientNum = (int)luaL_checkinteger(L, 1);
    float xp = (float)luaL_checknumber(L, 2);
    int skill = (int)luaL_checkinteger(L, 3);
    int add = (int)luaL_optinteger(L, 4, 0);
    gentity_t* ent;
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    ent = &g_entities[clientNum];
    if (!ent->client) {
        return 0;
    }
    
    if (skill < 0 || skill >= SK_NUM_SKILLS) {
        return 0;
    }
    
    if (add) {
        ent->client->sess.skillpoints[skill] += xp;
    } else {
        ent->client->sess.skillpoints[skill] = xp;
    }
    
    return 0;
}

// et.G_ResetXP(clientNum) - Reset XP for a player
static int _et_G_ResetXP(lua_State* L)
{
    int clientNum = (int)luaL_checkinteger(L, 1);
    gentity_t* ent;
    int i;
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    ent = &g_entities[clientNum];
    if (!ent->client) {
        return 0;
    }
    
    for (i = 0; i < SK_NUM_SKILLS; i++) {
        ent->client->sess.skillpoints[i] = 0;
        ent->client->sess.skill[i] = 0;
    }
    
    return 0;
}

// et.GetPlayerXP(clientNum, skill) - Get player XP for a skill
static int _et_GetPlayerXP(lua_State* L)
{
    int clientNum = (int)luaL_checkinteger(L, 1);
    int skill = (int)luaL_checkinteger(L, 2);
    gentity_t* ent;
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        lua_pushnil(L);
        return 1;
    }
    
    ent = &g_entities[clientNum];
    if (!ent->client) {
        lua_pushnil(L);
        return 1;
    }
    
    if (skill < 0 || skill >= SK_NUM_SKILLS) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_pushnumber(L, ent->client->sess.skillpoints[skill]);
    return 1;
}

// et.GetPlayerSkill(clientNum, skill) - Get player skill level
static int _et_GetPlayerSkill(lua_State* L)
{
    int clientNum = (int)luaL_checkinteger(L, 1);
    int skill = (int)luaL_checkinteger(L, 2);
    gentity_t* ent;
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        lua_pushnil(L);
        return 1;
    }
    
    ent = &g_entities[clientNum];
    if (!ent->client) {
        lua_pushnil(L);
        return 1;
    }
    
    if (skill < 0 || skill >= SK_NUM_SKILLS) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_pushinteger(L, ent->client->sess.skill[skill]);
    return 1;
}

// et.AddWeaponToPlayer(clientNum, weapon, ammo, ammoclip, setcurrent) - Add weapon to player
static int _et_AddWeaponToPlayer(lua_State* L)
{
    int clientNum = (int)luaL_checkinteger(L, 1);
    int weapon = (int)luaL_checkinteger(L, 2);
    int ammo = (int)luaL_optinteger(L, 3, 0);
    int ammoclip = (int)luaL_optinteger(L, 4, 0);
    int setcurrent = (int)luaL_optinteger(L, 5, 1);
    gentity_t* ent;
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    ent = &g_entities[clientNum];
    if (!ent->client) {
        return 0;
    }
    
    if (weapon < 0 || weapon >= WP_NUM_WEAPONS) {
        return 0;
    }
    
    COM_BitSet(ent->client->ps.weapons, weapon);
    ent->client->ps.ammoclip[weapon] = ammoclip;
    ent->client->ps.ammo[weapon] = ammo;
    
    if (setcurrent) {
        ent->client->ps.weapon = weapon;
    }
    
    return 0;
}

// et.RemoveWeaponFromPlayer(clientNum, weapon) - Remove weapon from player
static int _et_RemoveWeaponFromPlayer(lua_State* L)
{
    int clientNum = (int)luaL_checkinteger(L, 1);
    int weapon = (int)luaL_checkinteger(L, 2);
    gentity_t* ent;
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return 0;
    }
    
    ent = &g_entities[clientNum];
    if (!ent->client) {
        return 0;
    }
    
    if (weapon < 0 || weapon >= WP_NUM_WEAPONS) {
        return 0;
    }
    
    COM_BitClear(ent->client->ps.weapons, weapon);
    ent->client->ps.ammoclip[weapon] = 0;
    ent->client->ps.ammo[weapon] = 0;
    
    return 0;
}

// et.GetCurrentWeapon(clientNum) - Get player's current weapon
static int _et_GetCurrentWeapon(lua_State* L)
{
    int clientNum = (int)luaL_checkinteger(L, 1);
    gentity_t* ent;
    
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        lua_pushnil(L);
        return 1;
    }
    
    ent = &g_entities[clientNum];
    if (!ent->client) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_pushinteger(L, ent->client->ps.weapon);
    return 1;
}

// et.trap_PointContents(point, passent) - Get contents at a point
static int _et_trap_PointContents(lua_State* L)
{
    vec3_t point;
    int passent = (int)luaL_optinteger(L, 2, -1);
    
    _et_getvec3(L, 1, point);
    lua_pushinteger(L, trap_PointContents(point, passent));
    return 1;
}

// et.InPVS(point1, point2) - Check if two points are in PVS of each other
static int _et_InPVS(lua_State* L)
{
    vec3_t point1, point2;
    
    _et_getvec3(L, 1, point1);
    _et_getvec3(L, 2, point2);
    
    lua_pushboolean(L, trap_InPVS(point1, point2));
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Lua library registration
///////////////////////////////////////////////////////////////////////////////

static const luaL_Reg etlib[] = {
    // Printing
    { "G_Print",                 _et_G_Print                 },
    { "G_LogPrint",              _et_G_LogPrint              },
    
    // Module management
    { "RegisterModname",         _et_RegisterModname         },
    { "FindSelf",                _et_FindSelf                },
    { "FindMod",                 _et_FindMod                 },
    { "IPCSend",                 _et_IPCSend                 },
    
    // Cvars
    { "trap_Cvar_Get",           _et_trap_Cvar_Get           },
    { "trap_Cvar_Set",           _et_trap_Cvar_Set           },
    
    // Server commands
    { "trap_SendConsoleCommand", _et_trap_SendConsoleCommand },
    { "trap_SendServerCommand",  _et_trap_SendServerCommand  },
    
    // Arguments
    { "trap_Argc",               _et_trap_Argc               },
    { "trap_Argv",               _et_trap_Argv               },
    { "ConcatArgs",              _et_ConcatArgs              },
    
    // Config strings
    { "trap_GetConfigstring",    _et_trap_GetConfigstring    },
    { "trap_SetConfigstring",    _et_trap_SetConfigstring    },
    
    // Userinfo
    { "trap_GetUserinfo",        _et_trap_GetUserinfo        },
    { "trap_SetUserinfo",        _et_trap_SetUserinfo        },
    { "ClientUserinfoChanged",   _et_ClientUserinfoChanged   },
    
    // Client management
    { "trap_DropClient",         _et_trap_DropClient         },
    { "ClientNumberFromString",  _et_ClientNumberFromString  },
    { "G_Say",                   _et_G_Say                   },
    
    // String utilities
    { "Info_RemoveKey",          _et_Info_RemoveKey          },
    { "Info_SetValueForKey",     _et_Info_SetValueForKey     },
    { "Info_ValueForKey",        _et_Info_ValueForKey        },
    { "Q_CleanStr",              _et_Q_CleanStr              },
    
    // Filesystem
    { "trap_FS_FOpenFile",       _et_trap_FS_FOpenFile       },
    { "trap_FS_Read",            _et_trap_FS_Read            },
    { "trap_FS_Write",           _et_trap_FS_Write           },
    { "trap_FS_FCloseFile",      _et_trap_FS_FCloseFile      },
    { "trap_FS_Rename",          _et_trap_FS_Rename          },
    { "trap_FS_GetFileList",     _et_trap_FS_GetFileList     },
    
    // Sound
    { "G_SoundIndex",            _et_G_SoundIndex            },
    { "G_ModelIndex",            _et_G_ModelIndex            },
    { "G_globalSound",           _et_G_globalSound           },
    { "G_Sound",                 _et_G_Sound                 },
    
    // Entities
    { "G_Spawn",                 _et_G_Spawn                 },
    { "G_TempEntity",            _et_G_TempEntity            },
    { "G_FreeEntity",            _et_G_FreeEntity            },
    { "G_EntitiesFree",          _et_G_EntitiesFree          },
    { "G_AddEvent",              _et_G_AddEvent              },
    { "trap_LinkEntity",         _et_trap_LinkEntity         },
    { "trap_UnlinkEntity",       _et_trap_UnlinkEntity       },
    { "gentity_get",             _et_gentity_get             },
    { "gentity_set",             _et_gentity_set             },
    { "G_SetOrigin",             _et_G_SetOrigin             },
    { "G_SetAngles",             _et_G_SetAngles             },
    { "G_GetOrigin",             _et_G_GetOrigin             },
    { "G_GetAngles",             _et_G_GetAngles             },
    
    // Skills and XP
    { "G_AddSkillPoints",        _et_G_AddSkillPoints        },
    { "G_LoseSkillPoints",       _et_G_LoseSkillPoints       },
    { "G_XP_Set",                _et_G_XP_Set                },
    { "G_ResetXP",               _et_G_ResetXP               },
    { "GetPlayerXP",             _et_GetPlayerXP             },
    { "GetPlayerSkill",          _et_GetPlayerSkill          },
    
    // Weapons
    { "AddWeaponToPlayer",       _et_AddWeaponToPlayer       },
    { "RemoveWeaponFromPlayer",  _et_RemoveWeaponFromPlayer  },
    { "GetCurrentWeapon",        _et_GetCurrentWeapon        },
    
    // Tracing
    { "trap_Trace",              _et_trap_Trace              },
    { "trap_PointContents",      _et_trap_PointContents      },
    { "InPVS",                   _et_InPVS                   },
    
    // Miscellaneous
    { "trap_Milliseconds",       _et_trap_Milliseconds       },
    { "isBitSet",                _et_isBitSet                },
    { "G_Damage",                _et_G_Damage                },
    
    { NULL,                      NULL                        }
};

///////////////////////////////////////////////////////////////////////////////
// Helper macro for registering constants
///////////////////////////////////////////////////////////////////////////////

#define lua_regconstinteger(L, n) \
    (lua_pushstring(L, #n), lua_pushinteger(L, n), lua_settable(L, -3))

///////////////////////////////////////////////////////////////////////////////
// G_LuaRegisterConstants - Register game constants for Lua scripts
///////////////////////////////////////////////////////////////////////////////

static void G_LuaRegisterConstants(lua_State* L)
{
    // Get the et table
    lua_getglobal(L, "et");
    
    // Team constants
    lua_regconstinteger(L, TEAM_FREE);
    lua_regconstinteger(L, TEAM_AXIS);
    lua_regconstinteger(L, TEAM_ALLIES);
    lua_regconstinteger(L, TEAM_SPECTATOR);
    lua_regconstinteger(L, TEAM_NUM_TEAMS);
    
    // Class constants
    lua_regconstinteger(L, PC_SOLDIER);
    lua_regconstinteger(L, PC_MEDIC);
    lua_regconstinteger(L, PC_ENGINEER);
    lua_regconstinteger(L, PC_FIELDOPS);
    lua_regconstinteger(L, PC_COVERTOPS);
    
    // Skill constants
    lua_regconstinteger(L, SK_BATTLE_SENSE);
    lua_regconstinteger(L, SK_EXPLOSIVES_AND_CONSTRUCTION);
    lua_regconstinteger(L, SK_FIRST_AID);
    lua_regconstinteger(L, SK_SIGNALS);
    lua_regconstinteger(L, SK_LIGHT_WEAPONS);
    lua_regconstinteger(L, SK_HEAVY_WEAPONS);
    lua_regconstinteger(L, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS);
    lua_regconstinteger(L, SK_NUM_SKILLS);
    
    // Weapon constants
    lua_regconstinteger(L, WP_NONE);
    lua_regconstinteger(L, WP_KNIFE);
    lua_regconstinteger(L, WP_LUGER);
    lua_regconstinteger(L, WP_MP40);
    lua_regconstinteger(L, WP_GRENADE_LAUNCHER);
    lua_regconstinteger(L, WP_PANZERFAUST);
    lua_regconstinteger(L, WP_FLAMETHROWER);
    lua_regconstinteger(L, WP_COLT);
    lua_regconstinteger(L, WP_THOMPSON);
    lua_regconstinteger(L, WP_GRENADE_PINEAPPLE);
    lua_regconstinteger(L, WP_STEN);
    lua_regconstinteger(L, WP_MEDIC_SYRINGE);
    lua_regconstinteger(L, WP_AMMO);
    lua_regconstinteger(L, WP_ARTY);
    lua_regconstinteger(L, WP_SILENCER);
    lua_regconstinteger(L, WP_DYNAMITE);
    lua_regconstinteger(L, WP_MEDKIT);
    lua_regconstinteger(L, WP_BINOCULARS);
    lua_regconstinteger(L, WP_PLIERS);
    lua_regconstinteger(L, WP_SMOKE_MARKER);
    lua_regconstinteger(L, WP_KAR98);
    lua_regconstinteger(L, WP_CARBINE);
    lua_regconstinteger(L, WP_GARAND);
    lua_regconstinteger(L, WP_LANDMINE);
    lua_regconstinteger(L, WP_SATCHEL);
    lua_regconstinteger(L, WP_SATCHEL_DET);
    lua_regconstinteger(L, WP_SMOKE_BOMB);
    lua_regconstinteger(L, WP_MOBILE_MG42);
    lua_regconstinteger(L, WP_K43);
    lua_regconstinteger(L, WP_FG42);
    lua_regconstinteger(L, WP_MORTAR);
    lua_regconstinteger(L, WP_AKIMBO_COLT);
    lua_regconstinteger(L, WP_AKIMBO_LUGER);
    lua_regconstinteger(L, WP_GPG40);
    lua_regconstinteger(L, WP_M7);
    lua_regconstinteger(L, WP_SILENCED_COLT);
    lua_regconstinteger(L, WP_GARAND_SCOPE);
    lua_regconstinteger(L, WP_K43_SCOPE);
    lua_regconstinteger(L, WP_FG42SCOPE);
    lua_regconstinteger(L, WP_MORTAR_SET);
    lua_regconstinteger(L, WP_MEDIC_ADRENALINE);
    lua_regconstinteger(L, WP_AKIMBO_SILENCEDCOLT);
    lua_regconstinteger(L, WP_AKIMBO_SILENCEDLUGER);
    lua_regconstinteger(L, WP_MOBILE_MG42_SET);
    lua_regconstinteger(L, WP_NUM_WEAPONS);
    
    // Means of death constants
    lua_regconstinteger(L, MOD_UNKNOWN);
    lua_regconstinteger(L, MOD_MACHINEGUN);
    lua_regconstinteger(L, MOD_BROWNING);
    lua_regconstinteger(L, MOD_MG42);
    lua_regconstinteger(L, MOD_GRENADE);
    lua_regconstinteger(L, MOD_KNIFE);
    lua_regconstinteger(L, MOD_LUGER);
    lua_regconstinteger(L, MOD_COLT);
    lua_regconstinteger(L, MOD_MP40);
    lua_regconstinteger(L, MOD_THOMPSON);
    lua_regconstinteger(L, MOD_STEN);
    lua_regconstinteger(L, MOD_GARAND);
    lua_regconstinteger(L, MOD_SILENCER);
    lua_regconstinteger(L, MOD_FG42);
    lua_regconstinteger(L, MOD_FG42SCOPE);
    lua_regconstinteger(L, MOD_PANZERFAUST);
    lua_regconstinteger(L, MOD_GRENADE_LAUNCHER);
    lua_regconstinteger(L, MOD_FLAMETHROWER);
    lua_regconstinteger(L, MOD_GRENADE_PINEAPPLE);
    lua_regconstinteger(L, MOD_DYNAMITE);
    lua_regconstinteger(L, MOD_AIRSTRIKE);
    lua_regconstinteger(L, MOD_SYRINGE);
    lua_regconstinteger(L, MOD_AMMO);
    lua_regconstinteger(L, MOD_ARTY);
    lua_regconstinteger(L, MOD_WATER);
    lua_regconstinteger(L, MOD_SLIME);
    lua_regconstinteger(L, MOD_LAVA);
    lua_regconstinteger(L, MOD_CRUSH);
    lua_regconstinteger(L, MOD_TELEFRAG);
    lua_regconstinteger(L, MOD_FALLING);
    lua_regconstinteger(L, MOD_SUICIDE);
    lua_regconstinteger(L, MOD_TRIGGER_HURT);
    lua_regconstinteger(L, MOD_EXPLOSIVE);
    lua_regconstinteger(L, MOD_CARBINE);
    lua_regconstinteger(L, MOD_KAR98);
    lua_regconstinteger(L, MOD_GPG40);
    lua_regconstinteger(L, MOD_M7);
    lua_regconstinteger(L, MOD_LANDMINE);
    lua_regconstinteger(L, MOD_SATCHEL);
    lua_regconstinteger(L, MOD_SMOKEBOMB);
    lua_regconstinteger(L, MOD_MOBILE_MG42);
    lua_regconstinteger(L, MOD_SILENCED_COLT);
    lua_regconstinteger(L, MOD_GARAND_SCOPE);
    lua_regconstinteger(L, MOD_K43);
    lua_regconstinteger(L, MOD_K43_SCOPE);
    lua_regconstinteger(L, MOD_MORTAR);
    lua_regconstinteger(L, MOD_AKIMBO_COLT);
    lua_regconstinteger(L, MOD_AKIMBO_LUGER);
    lua_regconstinteger(L, MOD_AKIMBO_SILENCEDCOLT);
    lua_regconstinteger(L, MOD_AKIMBO_SILENCEDLUGER);
    lua_regconstinteger(L, MOD_SMOKEGRENADE);
    lua_regconstinteger(L, MOD_SWAP_PLACES);
    lua_regconstinteger(L, MOD_SWITCHTEAM);
    lua_regconstinteger(L, MOD_NUM_MODS);
    
    // Say mode constants
    lua_regconstinteger(L, SAY_ALL);
    lua_regconstinteger(L, SAY_TEAM);
    lua_regconstinteger(L, SAY_BUDDY);
    lua_regconstinteger(L, SAY_TEAMNL);
    
    // Exec constants
    lua_regconstinteger(L, EXEC_NOW);
    lua_regconstinteger(L, EXEC_INSERT);
    lua_regconstinteger(L, EXEC_APPEND);
    
    // Filesystem constants
    lua_regconstinteger(L, FS_READ);
    lua_regconstinteger(L, FS_WRITE);
    lua_regconstinteger(L, FS_APPEND);
    lua_regconstinteger(L, FS_APPEND_SYNC);
    
    // Max constants
    lua_regconstinteger(L, MAX_CLIENTS);
    lua_regconstinteger(L, MAX_GENTITIES);
    lua_regconstinteger(L, MAX_MODELS);
    lua_regconstinteger(L, MAX_SOUNDS);
    
    // Configstring constants
    lua_regconstinteger(L, CS_SERVERINFO);
    lua_regconstinteger(L, CS_SYSTEMINFO);
    lua_regconstinteger(L, CS_MUSIC);
    lua_regconstinteger(L, CS_MESSAGE);
    lua_regconstinteger(L, CS_MOTD);
    lua_regconstinteger(L, CS_WARMUP);
    lua_regconstinteger(L, CS_VOTE_TIME);
    lua_regconstinteger(L, CS_VOTE_STRING);
    lua_regconstinteger(L, CS_VOTE_YES);
    lua_regconstinteger(L, CS_VOTE_NO);
    lua_regconstinteger(L, CS_GAME_VERSION);
    lua_regconstinteger(L, CS_LEVEL_START_TIME);
    lua_regconstinteger(L, CS_INTERMISSION);
    lua_regconstinteger(L, CS_PLAYERS);
    
    // Pop the et table
    lua_pop(L, 1);
}

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
    
    // Register game constants
    G_LuaRegisterConstants(vm->L);

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
        unsigned long hash1 = 5381;
        unsigned long hash2 = 0;
        int c;
        char* p = code;
        int idx = 0;
        while ((c = *p++)) {
            hash1 = ((hash1 << 5) + hash1) + c; // hash * 33 + c
            hash2 = hash2 ^ (c * (idx + 1)); // secondary hash
            idx++;
        }
        
        // Also incorporate file length for uniqueness
        hash1 ^= (unsigned long)flen;
        hash2 ^= (unsigned long)(flen * 31);
        
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
        // Store hash as hex signature (two 32-bit values for portability)
        Com_sprintf(vm->mod_signature, sizeof(vm->mod_signature), "%08lX%08lX", 
            hash1 & 0xFFFFFFFF, hash2 & 0xFFFFFFFF);
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

// G_LuaHook_Obituary - Called when a player dies
void G_LuaHook_Obituary(int victim, int killer, int meansOfDeath)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_Obituary")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, victim);
            lua_pushinteger(vm->L, killer);
            lua_pushinteger(vm->L, meansOfDeath);
            // Call
            if (!G_LuaCall(vm, "et_Obituary", 3, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_Damage - Called when damage is dealt
// Returns qtrue if damage should be blocked
qboolean G_LuaHook_Damage(int target, int attacker, int damage, int dflags, int mod)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_Damage")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, target);
            lua_pushinteger(vm->L, attacker);
            lua_pushinteger(vm->L, damage);
            lua_pushinteger(vm->L, dflags);
            lua_pushinteger(vm->L, mod);
            // Call
            if (!G_LuaCall(vm, "et_Damage", 5, 1)) {
                continue;
            }
            // Return values - return 1 to block damage
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

// G_LuaHook_ChatMessage - Called when a player chats
// Returns qtrue if message should be blocked
qboolean G_LuaHook_ChatMessage(int clientNum, int mode, int chatType, char* message)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_ChatMessage")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            lua_pushinteger(vm->L, mode);
            lua_pushinteger(vm->L, chatType);
            lua_pushstring(vm->L, message);
            // Call
            if (!G_LuaCall(vm, "et_ChatMessage", 4, 1)) {
                continue;
            }
            // Return values - return 1 to block message
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

// G_LuaHook_WeaponFire - Called when a weapon is fired
// Returns qtrue if firing should be blocked
qboolean G_LuaHook_WeaponFire(int clientNum, int weapon)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_WeaponFire")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            lua_pushinteger(vm->L, weapon);
            // Call
            if (!G_LuaCall(vm, "et_WeaponFire", 2, 1)) {
                continue;
            }
            // Return values - return 1 to block firing
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

// G_LuaHook_Revive - Called when a player is revived
void G_LuaHook_Revive(int clientNum, int reviverNum)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_Revive")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            lua_pushinteger(vm->L, reviverNum);
            // Call
            if (!G_LuaCall(vm, "et_Revive", 2, 0)) {
                continue;
            }
        }
    }
}

// G_LuaHook_SetPlayerSkill - Called when a player's skill is set
// Returns qtrue if skill change should be blocked
qboolean G_LuaHook_SetPlayerSkill(int clientNum, int skill, int level)
{
    int i;
    lua_vm_t* vm;

    for (i = 0; i < LUA_NUM_VM; i++) {
        vm = lVM[i];
        if (vm) {
            if (vm->id < 0) {
                continue;
            }
            if (!G_LuaGetNamedFunction(vm, "et_SetPlayerSkill")) {
                continue;
            }
            // Arguments
            lua_pushinteger(vm->L, clientNum);
            lua_pushinteger(vm->L, skill);
            lua_pushinteger(vm->L, level);
            // Call
            if (!G_LuaCall(vm, "et_SetPlayerSkill", 3, 1)) {
                continue;
            }
            // Return values - return 1 to block skill change
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
