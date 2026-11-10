# Simple Steam Input plugin

This plugin is developed for the use case of detecting controller types from steam input, and exposing this functionality as a blueprint
call for Unreal Engine.

## Usage

### GetConnectedControllers
<img width="304" height="166" alt="GetConnectedControllers" src="https://github.com/user-attachments/assets/c6b8e575-2b88-4c35-b7f1-a1f4f6d287ba" />

### Available controller types
<img width="514" height="363" alt="Available controller types" src="https://github.com/user-attachments/assets/c7052753-a4ff-4f88-a7ce-d0ad861b31e7" />

### DualSense adaptive triggers and lightbar colour (per player)
Blueprint functions under **Steam Input > Feedback**. They take a Player Controller and do nothing for pads without adaptive
triggers or a lightbar, keyboard players and games without Steam, so it's safe to call them for everyone.

| Node | What it does |
|---|---|
| `Set Trigger Preset (Player, Trigger, Preset, Duration)` | `Off`, `Light`, `Heavy`, `Locked`, `Weapon` (a click part way), `Ramp` (builds up), `Vibration` |
| `Set Trigger Effect (Player, Trigger, Effect, Duration)` | A custom effect: Feedback, Weapon, Vibration, Slope Feedback, Multiple Position Feedback/Vibration |
| `Make Trigger Effect From Preset (Preset)` | A preset's values, as a starting point for a custom effect |
| `Reset Trigger Effects (Player)` / `Reset All Trigger Effects` | Back to Off |
| `Has Adaptive Triggers (Player)` | True when the player holds a DualSense |
| `Set Lightbar Color (Player, Color, Duration)` | DualShock 4 / DualSense light colour |
| `Reset Lightbar (Player)` / `Reset All Lightbars` | Back to the colour the player chose in Steam |
| `Has Lightbar (Player)` | True for a DualShock 4 or DualSense |
| `Get Steam Gamepad Index (Player)` | For the index-based Steam Input Subsystem calls (e.g. rumble) |

- **Trigger:** `Left` (L2/LT), `Right` (R2/RT) or `Both`. Setting one trigger leaves the other alone.
- **Duration 0** sets a lasting effect. **Duration > 0** sets a timed one, which then falls back to the lasting effect.
  For example, `Heavy` on the right trigger while a weapon is equipped, then `Locked` for 1.5 s on each shot (a cooldown).
- **Lightbar:** Duration 0 sets a lasting colour, which stays across levels. Duration > 0 is a flash (e.g. on a hit)
  that returns to the lasting colour, or to the player's Steam colour if none was set.
- `Locked` is the strongest resistance a DualSense can apply from the start of the pull. A determined player can still
  push through it.
- **Automatic resets:**
  - Triggers are released while the game is paused or the Steam overlay is open, and restored afterwards. Turn this off
    with `Release Triggers While Paused` on the Steam Input Feedback Subsystem.
  - They're reset on level change and on exit.
  - Trigger effects and colours are re-sent when a pad reconnects.
  - Lightbars go back to each player's Steam colour on exit.
- **C++:** `USteamInputFeedbackSubsystem` (game instance subsystem), keyed by gamepad index (the local player's
  `ControllerId`).

## Testing

### Unreal Editor

- Add the unreal editor on steam as a non steam game

<img width="332" height="139" alt="image" src="https://github.com/user-attachments/assets/189ad1c3-6f17-4288-9558-b175d4ef22b6" />

- Place a `steam_appid.txt` file (exactly that name: not `steam_app_id.txt`) in the START IN directory
- if you are not sure the current working directory is working properly, when you open unreal, open the python repl and do
```python
import os
os.getcwd()
```
- The contents of `steam_appid.txt` should be
```
480
```
That's an ID of a test game called Spacewar
- Start the Unreal Editor from steam, connect your blueprints and add some print string blocks to verify the controllers are connected properly.
- When unreal exits, the `steam_appid.txt` will vanish, so either change the working directory or be careful to recreate it everytime you terminate Unreal Editor.

### Game build
- Similar to the unreal editor, add a custom build on steam like so
<img width="838" height="460" alt="image" src="https://github.com/user-attachments/assets/bdb37c44-3351-435a-902c-393f7a8b74e5" />

- Place the same `steam_appid.txt` as in the Unreal Editor step, in the START IN directory of the shortcut (e.g. the packaged
  build's root folder, next to `<Game>.sh`). Use a Development build if you want logs: Shipping builds write none.
- Launch the game, again Spacewar will show as being played on your steam account.

### Steamworks
- Make sure your Steam Input section is configured properly

<img width="1326" height="527" alt="image" src="https://github.com/user-attachments/assets/5ac0a874-6e3a-4e64-8154-94765b468b7c" />

- And when you see your game on your Steam Library, the steam input is enabled

<img width="849" height="607" alt="image" src="https://github.com/user-attachments/assets/580f2957-5242-4774-ac6a-44b8933ac896" />


## Future development

- This plugin was created as a utility for our upcoming video game, [Cuprum 2929](https://store.steampowered.com/app/2660240/CUPRUM2929/) and the roadmap of the plugin is closely connected to our specific needs. Feel free to contribute if you'd like, we are open to accepting PRs to complete the blueprint integration with the rest of steam input functionality
