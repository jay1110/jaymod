#ifndef __LUACONTEXT_H
#define __LUACONTEXT_H

#include <base/public.h>
#include <base/lua/public.h>

class LuaContext
{
private:
	lua_State* state;
	string errorMsg;

public:
	// Constructor: Creates a new Lua state.
	// If loadStdLibs is true (default), all Lua standard libraries are loaded.
	// Use isValid() to check if the state was created successfully.
	LuaContext(bool loadStdLibs = true);

	// Destructor: Closes the Lua state
	~LuaContext();

	// Execute a Lua string. Returns true on success, false on error.
	bool doString(const string& code);

	// Execute a Lua file. Returns true on success, false on error.
	bool doFile(const string& filename);

	// Get the last error message
	const string& getErrorMsg() const;

	// Get the underlying Lua state for advanced usage
	lua_State* getState() const;

	// Check if the Lua state is valid
	bool isValid() const;

	// Register a C function to be called from Lua
	void registerFunction(const char* name, lua_CFunction func);

	// Get a global integer value
	int getGlobalInt(const char* name, int defaultValue = 0);

	// Get a global string value
	string getGlobalString(const char* name, const string& defaultValue = "");

	// Set a global integer value
	void setGlobalInt(const char* name, int value);

	// Set a global string value
	void setGlobalString(const char* name, const string& value);
};

#endif // LUACONTEXT_H
