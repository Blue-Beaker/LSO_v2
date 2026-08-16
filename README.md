# LevelSongOffset
### UNFINISHED YET, but the main functionality is already available, download from actions if you want to try
Known problem: When set offset to negative, Trigger a song trigger (not prep) and pause before the song actually plays, then resume, the music will start immediately, ignoring the offset.


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
