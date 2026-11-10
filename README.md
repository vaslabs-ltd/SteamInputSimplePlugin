# Simple Steam Input plugin

This plugin is developed for the use case of detecting controller types from steam input, and exposing this functionality as a blueprint
call for Unreal Engine.

## Usage

### GetConnectedControllers
<img width="304" height="166" alt="GetConnectedControllers" src="https://github.com/user-attachments/assets/c6b8e575-2b88-4c35-b7f1-a1f4f6d287ba" />

### Available controller types
<img width="514" height="363" alt="Available controller types" src="https://github.com/user-attachments/assets/c7052753-a4ff-4f88-a7ce-d0ad861b31e7" />

### Controller glyphs (button prompts per player)
Show each player the buttons of the pad they're actually holding. The family comes from Steam Input, matching the local
player's `ControllerId` to Steam's gamepad index.

- **Families:** Xbox, PlayStation, Switch (a Pro Controller or a Joy-Con pair), SwitchJoyCon (one Joy-Con held sideways),
  SteamDeck and SteamController. Generic pads, and every pad when Steam isn't running, show as Xbox.
- **Prompt tokens** use the Xbox names: `{A} {B} {X} {Y} {LB} {RB} {LT} {RT} {Start} {Back} {DPadLR} {DPadUD} {DPad}
  {DPadUp} {DPadDown} {DPadLeft} {DPadRight} {LS} {RS} {L3} {R3}`. For example, `Press {A} to Join` shows a cross on a
  PlayStation pad and a green A on an Xbox pad.
- Each token becomes rich text, `<Glyph.<Family>.<Button>>char</>`, drawn with the matching glyph font.
  - Family is `Xbox`, `PS`, `Switch`, `SwitchJoyCon`, `SteamDeck` or `SteamController`.
  - A family without a glyph for a button falls back to Xbox's.

**The easy way: the "Steam Input Prompt Text" widget** (UMG palette, under Steam Input)
- It's a Rich Text Block with a **Prompt Template** field (e.g. `Press {A} to Join`). It formats the template with the
  player's glyphs and redraws itself when controllers change, so no event wiring is needed.
- Set **Text Style Set** to `/SteamInputSimplePlugin/Content/DT_SteamInputGlyphs`, or to a Composite Data Table that includes it
  along with your own text styles.
- **Player:** `Set Prompt Player` (per-player cards), otherwise the widget's owning player, otherwise the first local
  player. Use `Set Prompt Template` to change the text at runtime.

**Blueprint functions** (Steam Input > Prompts)

| Node | What it does |
|---|---|
| `Format Controller Prompt (Template, Owning Player)` | The template with that player's glyphs (the first local player's when empty) |
| `Format Controller Prompt For Family (Template, Family)` | The template with a given family's glyphs |
| `Get Controller Family (Owning Player)` | Xbox, PlayStation, Switch, ... |
| `Get Controller Texture (Xbox Texture, Owning Player)` | The family's version of an image with buttons drawn in |

- **Event:** `On Controller Families Changed`, on the Steam Input Prompt Subsystem, fires shortly after a pad connects
  or disconnects.
- **C++:** `USteamInputPromptSubsystem::FormatPrompt(Template, Family)` / `FormatPromptFor(Template, OwnerPC)`. Apply it
  **before** `FText::Format` with `{0}`-style arguments, because ordered formatting turns `{A}` into "0".
- **Switch layout:** Switch glyphs follow Steam's per-player "Use Nintendo Button Layout" setting. `{A}` shows A when
  Steam maps buttons by label, and B (the bottom button) when it maps by position. You can force either in the settings.
- **Settings:** Project Settings > Plugins > Steam Input Prompts. You can change the glyph per family and button, with an
  optional style row override, the row prefix and the Switch layout. **Images** maps an Xbox texture to its version per
  family, for `Get Controller Texture`.
- **Assets:**
  - Glyph fonts: `/SteamInputSimplePlugin/Content/UI/Fonts/F_KenneyInput*` (one per family).
  - Style table: `DT_SteamInputGlyphs`, imported from `Resources/KenneyInputPrompts/DT_SteamInputGlyphs.csv`.
  - To regenerate the table (e.g. a different size), run `Resources/KenneyInputPrompts/make_glyph_style_csv.py [--size 69]`
    and reimport the CSV.

### Auto-pause
The Steam Input Pause Subsystem has one event, `On Pause Requested (Reason, Player Index)`. It fires when a local player's
controller disconnects (with that player's index), or when the Steam overlay opens (with -1). The game decides whether
and how to pause.

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
  - Steam re-applies its own pad settings when it switches configs (e.g. the game losing and regaining focus), so
    colours and trigger effects are re-sent then too: on focus, when the overlay closes, when UE's pads reconnect, and
    again 0.5 s and 2 s later.
- **Lightbar testing:** with the Spacewar (480) + non-Steam shortcut setup below, Steam drops the colour, while
  trigger effects still work. Steam activates the shortcut's config, not 480's. Launch the game under its own app ID,
  from your Steam library (e.g. a playtest or a beta branch), and the lightbar works (confirmed 2026-11-10).
- **Debugging:** `SteamInput.SetLightbar <GamepadIndex> <R> <G> <B>` (or `... default`) in the console sends a colour
  straight to Steam. `LogSteamInputFeedback` logs every colour requested and sent.
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


## Credits
- Controller glyphs: [Input Prompts](https://kenney.nl/assets/input-prompts) by [Kenney](https://www.kenney.nl), licensed
  [CC0](http://creativecommons.org/publicdomain/zero/1.0/). The fonts, their character maps and Kenney's licence are in
  `Plugins/SteamInputSimplePlugin/Resources/KenneyInputPrompts`. Thank you, Kenney! If they're useful to you too,
  consider [supporting Kenney](https://www.kenney.nl/donate).

## Future development

- This plugin was created as a utility for our upcoming video game, [Cuprum 2929](https://store.steampowered.com/app/2660240/CUPRUM2929/) and the roadmap of the plugin is closely connected to our specific needs. Feel free to contribute if you'd like, we are open to accepting PRs to complete the blueprint integration with the rest of steam input functionality
