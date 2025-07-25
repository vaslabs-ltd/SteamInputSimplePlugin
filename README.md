# Simple Steam Input plugin

This plugin is developed for the use case of detecting controller types from steam input, and exposing this functionality as a blueprint
call for Unreal Engine.

## Usage

### GetConnectedControllers
<img width="304" height="166" alt="GetConnectedControllers" src="https://github.com/user-attachments/assets/c6b8e575-2b88-4c35-b7f1-a1f4f6d287ba" />

### Available controller types
<img width="514" height="363" alt="Available controller types" src="https://github.com/user-attachments/assets/c7052753-a4ff-4f88-a7ce-d0ad861b31e7" />

## Testing

### Unreal Editor

- Add the unreal editor on steam as a non steam game

<img width="332" height="139" alt="image" src="https://github.com/user-attachments/assets/189ad1c3-6f17-4288-9558-b175d4ef22b6" />

- Place a steam_appid.txt file in the start in workspace
- if you are not sure the current working directory is working properly, when you open unreal, open the python repl and do
```python
import os
os.getcwd()
```
- The contents of steam_app_id.txt should be
```
480
```
That's an ID of a test game called Spacewar
- Start the Unreal Editor from steam, connect your blueprints and add some print string blocks to verify the controllers are connected properly.
- When unreal exits, the steam_appid.txt will vanish, so either change the working directory or be careful to recreate it everytime you terminate Unreal Editor.

### Game build
- Similar to the unreal editor, add a custom build on steam like so
<img width="838" height="460" alt="image" src="https://github.com/user-attachments/assets/bdb37c44-3351-435a-902c-393f7a8b74e5" />

- Place the same steam_appid.txt as in the unreal enditor step, but this time in the START IN directory
- Laungh the game, again Spacewar will show as being played on your steam account.

### Steamworks
- Make sure your Steam Input section is configured properly

<img width="1326" height="527" alt="image" src="https://github.com/user-attachments/assets/5ac0a874-6e3a-4e64-8154-94765b468b7c" />

- And when you see your game on your Steam Library, the steam input is enabled

<img width="849" height="607" alt="image" src="https://github.com/user-attachments/assets/580f2957-5242-4774-ac6a-44b8933ac896" />


## Future development

- This plugin was created as a utility for our upcoming video game, [Cuprum 2929](https://store.steampowered.com/app/2660240/CUPRUM2929/) and the roadmap of the plugin is closely connected to our specific needs. Feel free to contribute if you'd like, we are open to accepting PRs to complete the blueprint integration with the rest of steam input functionality
