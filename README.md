# Jaymod

Jaymod is a popular server-side modification for **Wolfenstein: Enemy Territory** that adds extensive gameplay features, admin tools, and customization options. This repository contains version 2.0.0.

## Table of Contents

- [Features](#features)
  - [Gameplay Features](#gameplay-features)
  - [Admin System](#admin-system)
  - [Lua Scripting](#lua-scripting)
  - [Bot Support](#bot-support)
- [Installation](#installation)
- [Server Configuration](#server-configuration)
  - [Key CVARs](#key-cvars)
  - [Game Modes](#game-modes)
- [Admin Commands](#admin-commands)
- [Building from Source](#building-from-source)
- [Platform Support](#platform-support)
- [License](#license)

---

## Features

### Gameplay Features

| Feature | Description | CVAR |
|---------|-------------|------|
| **XP Save** | Saves player XP across map changes and reconnects | `g_xpSave` |
| **Killing Spree** | Announces killing sprees and tracks records | `g_killingSpree` |
| **Goomba Kills** | Damage from landing on other players (Mario-style) | `g_goomba` |
| **Player Shove** | Push teammates out of your way | `g_shove` |
| **Corpse Dragging** | Drag dead bodies to safety for revival | `g_dragCorpse` |
| **Play Dead** | Fake death to surprise enemies | `g_playDead` |
| **Poison Syringes** | Medics can poison enemies with special syringes | `g_poisonSyringes` |
| **Anti-Warp** | Prevents warping exploits from laggy players | `g_antiwarp` |
| **Private Messages** | Send private messages between players | `g_privateMessages` |
| **Banners** | Rotating server messages | `g_banners` |
| **Watermark** | Display server watermark on screen | `g_watermark` |

### Fun Game Modes

| Mode | Description | CVAR |
|------|-------------|------|
| **Panzer War** | All players spawn with Panzerfausts, infinite ammo, reduced damage | `g_panzerWar` |
| **Sniper War** | All players spawn as Covert Ops with sniper rifles, instant headshot kills | `g_sniperWar` |
| **Knife Only** | Melee-only combat | `g_knifeonly` |

### Admin System

Jaymod includes a powerful admin system with multi-level permissions:

- **Level-based administration** - Assign different permission levels to admins
- **User database** - Persistent storage of admin accounts and bans
- **Ban system** - IP and GUID-based banning with duration support
- **Admin logging** - Track all admin actions

Enable with: `g_admin 1`

### Lua Scripting

Lua 5.4.7 is fully integrated for server-side scripting:

- Create custom game logic
- Extend admin commands
- Add new gameplay features
- Supports up to 64 Lua VMs simultaneously

**Supported Platforms:**
- Linux 32-bit (i386)
- Linux 64-bit (x86_64)
- Windows 32-bit (x86)
- Windows 64-bit (x64)

Lua is compiled as a static library—no external installation required.

### Bot Support

Full **Omnibot** integration for AI-controlled players:

- Enable with: `omnibot_enable 1`
- Requires Omnibot library files (not included)

---

## Installation

1. Download the latest release from the [Releases](../../releases) page
2. Extract files to your ET server's mod folder:
   ```
   /path/to/enemy-territory/xmod/
   ```
3. Start your server with `+set fs_game xmod`

### Required Files

```
xmod/
├── xmod-2.0.0.pk3      # Client-side assets (cgame, ui)
├── qagame.mp.*.so      # Linux server module
├── qagame_mp_*.dll     # Windows server module
├── xmod.cfg            # Mod configuration
└── server.cfg          # Server configuration
```

---

## Server Configuration

### Key CVARs

#### Gameplay Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `g_xpSave` | 0 | XP save mode (0=off, 1=on, 2=reset on campaign) |
| `g_xpSaveTimeout` | 0 | Time in seconds before saved XP expires |
| `g_xpMax` | 0 | Maximum XP a player can earn (0=unlimited) |
| `g_killingSpree` | 0 | Killing spree announcements (0=off, 1=on, 2=record) |
| `g_goomba` | 0 | Goomba damage multiplier (0=disabled) |
| `g_shove` | 0 | Shove distance (0=disabled, try 100) |
| `g_dragCorpse` | 0 | Enable corpse dragging |
| `g_playDead` | 0 | Enable play dead feature |
| `g_poisonSyringes` | 0 | Enable poison syringes |
| `g_antiwarp` | 1 | Anti-warp protection flags |

#### Team Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `team_maxMedics` | -1 | Max medics per team (-1=unlimited) |
| `team_maxEngineers` | -1 | Max engineers per team |
| `team_maxFieldOps` | -1 | Max field ops per team |
| `team_maxCovertOps` | -1 | Max covert ops per team |
| `team_maxPanzers` | -1 | Max players with Panzerfaust |
| `team_maxMortars` | -1 | Max players with mortar |

#### Admin Settings

| CVAR | Default | Description |
|------|---------|-------------|
| `g_admin` | 0 | Enable admin system |
| `g_adminLog` | "" | Admin action log file |
| `g_kickTime` | 120 | Default kick duration (seconds) |
| `g_muteTime` | 60 | Default mute duration (seconds) |

---

## Admin Commands

Admin commands use the `!` prefix (e.g., `!kick player`).

### Player Management

| Command | Description | Usage |
|---------|-------------|-------|
| `!kick` | Kick a player | `!kick <player> [reason]` |
| `!ban` | Ban a player | `!ban <player> [duration] [reason]` |
| `!unban` | Remove a ban | `!unban <ban#>` |
| `!mute` | Mute a player | `!mute <player> [duration]` |
| `!unmute` | Unmute a player | `!unmute <player>` |
| `!putteam` | Move player to team | `!putteam <player> <r/b/s>` |
| `!spec` | Move player to spectator | `!spec <player>` |
| `!rename` | Rename a player | `!rename <player> <newname>` |

### Fun Commands

| Command | Description | Usage |
|---------|-------------|-------|
| `!slap` | Slap a player (damage + knockback) | `!slap <player>` |
| `!smite` | Kill a player with lightning | `!smite <player>` |
| `!fling` | Launch a player into the air | `!fling <player>` |
| `!throw` | Throw a player in view direction | `!throw <player>` |
| `!splat` | Flatten a player | `!splat <player>` |
| `!shake` | Shake a player's screen | `!shake <player>` |
| `!disorient` | Disorient a player's controls | `!disorient <player>` |
| `!glow` | Make a player glow | `!glow <player> <color>` |
| `!pants` | Remove a player's pants | `!pants <player>` |
| `!chicken` | Turn player into a chicken | `!chicken <player>` |
| `!crybaby` | Make player cry | `!crybaby <player>` |

### Server Management

| Command | Description | Usage |
|---------|-------------|-------|
| `!nextmap` | Skip to next map | `!nextmap` |
| `!restart` | Restart current map | `!restart` |
| `!reset` | Reset match | `!reset` |
| `!pause` | Pause the game | `!pause` |
| `!unpause` | Unpause the game | `!unpause` |
| `!lock` | Lock a team | `!lock <r/b>` |
| `!unlock` | Unlock a team | `!unlock <r/b>` |
| `!shuffle` | Shuffle teams by XP | `!shuffle` |
| `!swap` | Swap teams | `!swap` |

### Level Management

| Command | Description | Usage |
|---------|-------------|-------|
| `!setlevel` | Set player's admin level | `!setlevel <player> <level>` |
| `!listplayers` | List all players | `!listplayers` |
| `!levlist` | List admin levels | `!levlist` |
| `!levinfo` | Show level details | `!levinfo <level>` |
| `!levadd` | Add admin level | `!levadd <level> <name>` |
| `!levdelete` | Delete admin level | `!levdelete <level>` |

### Information Commands

| Command | Description | Usage |
|---------|-------------|-------|
| `!help` | Show command help | `!help [command]` |
| `!about` | Show mod information | `!about` |
| `!status` | Show server status | `!status` |
| `!time` | Show server time | `!time` |
| `!uptime` | Show server uptime | `!uptime` |
| `!finger` | Show player info | `!finger <player>` |
| `!seen` | When player was last seen | `!seen <name>` |

---

## Building from Source

### Prerequisites

- GCC/G++ (Linux) or MinGW-w64 (Windows cross-compile)
- GNU Make 3.80+
- Python 3.x
- GNU M4

### Build Commands

```bash
# Linux 64-bit
PLATFORM=linux64 make release

# Linux 32-bit (requires gcc-multilib g++-multilib)
PLATFORM=linux make release

# Windows 32-bit (cross-compile via MinGW)
PLATFORM=mingw make release

# Windows 64-bit (cross-compile via MinGW)
PLATFORM=mingw64 make release
```

### Build Targets

| Target | Description |
|--------|-------------|
| `make` | Build default variant |
| `make release` | Build release variant |
| `make debug` | Build debug variant |
| `make clean` | Clean build output |
| `make pkg` | Build and package |

For detailed build system documentation, see [notes/BuildSystem.txt](notes/BuildSystem.txt).

---

## Platform Support

| Platform | Architecture | Build Command | Output |
|----------|--------------|---------------|--------|
| Linux | i386 (32-bit) | `PLATFORM=linux make` | `qagame.mp.i386.so` |
| Linux | x86_64 (64-bit) | `PLATFORM=linux64 make` | `qagame.mp.x86_64.so` |
| Windows | x86 (32-bit) | `PLATFORM=mingw make` | `qagame_mp_x86.dll` |
| Windows | x64 (64-bit) | `PLATFORM=mingw64 make` | `qagame_mp_x64.dll` |

All platforms include full Lua 5.4.7 integration.

---

## License

This source is bound to the original terms from **id Software**. Additionally, this source is released under the **Apache 2.0 License**.

Feel free to use this codebase as you please, as long as both licenses are bundled and proper credit is given.

---

## Credits

- **Original Jaymod** - Jaybird
- **Current Maintainer** - jay1110
- **Anti-Warp Code** - Zinx (June 2007)
- **Lua Integration** - Based on ET Legacy implementation

## Links

- [GitHub Repository](https://github.com/jay1110/jaymod)
- [Wolfenstein: Enemy Territory](https://www.splashdamage.com/games/wolfenstein-enemy-territory/)

