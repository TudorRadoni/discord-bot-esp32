# Discord Bot ESP32 Project

A modular Discord bot implementation for ESP32 microcontrollers with integrated NeoPixel LED control and extensible command system.

## 🏗️ Architecture

This project follows clean software design principles with clear separation of concerns:

### Core Components

- **`SystemManager`** - Main system coordinator and initialization
- **`DiscordClient`** - Handles all Discord API communication
- **`NeoPixelManager`** - Manages LED animations and color control
- **`CommandSystem`** - Extensible command registration and execution system

### File Structure

```txt
├── include/
│   ├── config.h              # Your credentials (gitignored)
│   ├── config.h.template     # Template for setup
│   ├── SystemManager.h       # Main system coordinator
│   ├── DiscordClient.h       # Discord API communication
│   ├── NeoPixelManager.h     # LED control and animations
│   └── CommandSystem.h       # Command system interface
├── src/
│   ├── config.cpp            # Your credentials (gitignored)
│   ├── main.cpp              # Entry point (minimal)
│   ├── SystemManager.cpp     # System initialization & coordination
│   ├── DiscordClient.cpp     # Discord API implementation
│   ├── NeoPixelManager.cpp   # LED control implementation
│   └── CommandSystem.cpp     # Command handling logic
└── README.md
```

## ✨ Features

### 🌈 LED Control

- **Rainbow Mode**: Continuous color cycling (default)
- **Static Colors**: Red, green, blue, white
- **Smart Feedback**: Visual confirmation for commands
- **Power Control**: Complete on/off functionality

### 🎛️ Command System

- **Modular Design**: Easy to add new commands
- **Callback Architecture**: Clean separation of command logic
- **Case Insensitive**: Works with or without `/` prefix
- **Built-in Help**: Automatic command documentation

### 📡 Discord Integration

- **Real-time WebSocket**: Uses Discord Gateway API for instant message reception
- **Rich Responses**: Emoji-enhanced status messages
- **Error Handling**: Graceful failure recovery with auto-reconnection
- **SSL Security**: Secure WebSocket and HTTPS communication
- **Heartbeat System**: Maintains persistent connection to Discord

## Setup Instructions

### 1. Configuration Setup

Before building and uploading the project, you need to create your configuration files:

```bash
cp include/config.h.template include/config.h
cp src/config.cpp.template src/config.cpp
```

Edit both `include/config.h` (if needed) and `src/config.cpp` with your actual credentials:

- `YOUR_WIFI_SSID`: Your WiFi network name
- `YOUR_WIFI_PASSWORD`: Your WiFi password
- `YOUR_DISCORD_BOT_TOKEN`: Your Discord bot token
- `YOUR_DISCORD_CHANNEL_ID`: Your Discord channel ID

**Note**: Only edit `src/config.cpp` for the actual values. The header file `include/config.h` should remain as extern declarations.

### 2. Getting Discord Credentials

#### Discord Bot Token

1. Go to the [Discord Developer Portal](https://discord.com/developers/applications)
2. Create a new application or select an existing one
3. Go to the "Bot" section
4. Click "Reset Token" and copy the new token
5. **Important**: Keep this token secret and never share it publicly

#### Discord Channel ID

1. Enable Developer Mode in Discord (User Settings → Advanced → Developer Mode)
2. Right-click on the channel you want to use
3. Select "Copy Channel ID"

### 3. Security Notes

- Both `config.h` and `config.cpp` files are ignored by git to prevent accidental exposure of credentials
- Never commit your actual configuration files to version control
- If you suspect your bot token has been compromised, regenerate it immediately in the Discord Developer Portal
- The template files show the structure but contain placeholder values

### 4. Building and Uploading

Use PlatformIO to build and upload the project:

```bash
platformio run --target upload
```

## 🎮 Available Commands

| Command    | Description           | LED Effect    |
| ---------- | --------------------- | ------------- |
| `status`   | Show system status    | None          |
| `turn_on`  | Simulate PC power on  | Green flash   |
| `turn_off` | Simulate PC power off | Red flash     |
| `rainbow`  | Enable rainbow mode   | Rainbow cycle |
| `red`      | Set LED to red        | Solid red     |
| `green`    | Set LED to green      | Solid green   |
| `blue`     | Set LED to blue       | Solid blue    |
| `white`    | Set LED to white      | Solid white   |
| `off`      | Turn off LED          | LED off       |
| `help`     | Show all commands     | None          |

### Usage Tips

- Commands are case-insensitive
- You can use commands with or without `/` prefix
- LED starts in rainbow mode by default
- All commands provide Discord feedback

## 🔧 Hardware Requirements

- ESP32 development board
- NeoPixel LED (WS2812B compatible)
- USB cable for programming
- WiFi network connection

### Wiring

- NeoPixel Data → GPIO 2
- NeoPixel Power Enable → GPIO 4
- NeoPixel VCC → 3.3V or 5V
- NeoPixel GND → GND

## 🚀 Extending the System

### Adding New Commands

1. **Declare the callback** in `CommandSystem.h`:

   ```cpp
   static void myNewCommand();
   ```

2. **Implement the function** in `CommandSystem.cpp`:

    ```cpp
    void CommandSystem::myNewCommand() {
        // Your command logic here
        discordClient.sendMessage("Command executed!");
    }
    ```

3. **Register the command** in `SystemManager.cpp`:

    ```cpp
    commandSystem.addCommand("mycommand", "Description", CommandSystem::myNewCommand);
    ```

### Adding New LED Effects

Extend `NeoPixelManager` with new animation methods and call them from the update loop or command callbacks.

## 🏆 Design Principles Applied

- **Single Responsibility Principle**: Each class has one clear purpose
- **Dependency Inversion**: High-level modules don't depend on low-level details
- **Open/Closed Principle**: Easy to extend with new commands without modifying existing code
- **Interface Segregation**: Clean, focused interfaces for each component
- **DRY (Don't Repeat Yourself)**: Common functionality centralized
- **Separation of Concerns**: UI, business logic, and hardware control are separated

This architecture makes the codebase maintainable, testable, and easily extensible for future features!
