# LevelSongOffset

Level-specific song offset changer.  

Also includes a workaround for negative audio offset:  
Negative audio offset doesn't always work on GD. Sometimes it just doesn't work and breaks music after respawning from the start.  
This mod fixes it by delaying the music when the start is negative.  

<img src="logo.png" width="150" alt="the mod's logo" />

*Update logo.png to change your mod's icon (please)*

## Build instructions
Just follow geode instructions.
[geode docs](https://docs.geode-sdk.org/getting-started/create-mod#build)
```sh
# Assuming you have the Geode CLI set up already
geode build
```
