This is the source code of "Yet another PS2 Half-Life PC port" in form of Visual Studio 2015 project.

It contains some fixes and hacks to the original code as well as new entities:
"item_healthcharger"	- health charger
"item_recharge"		- HEV charger
"item_eyescanner"	- iris scanner
"item_generic"		- model without collision
"env_warpball"		- warp ball effect + monster maker
"trigger_playerfreeze"	- freezes/unfreezes player
"trigger_random"	- picks random target
"trigger_player_islave"	- detects "alien mode"
and Blue Shift entities: vest, helmet, Rosenberg (for PS2 mod devs)

All changes to the original code were marked with "PS2HL" comments. File names of a new entities are starting with "ps2hl_" prefix.

Credits:
Based on HL SDK v2.3 fix for Visual Studio 2008 by Gary_McTaggart
Code for Blue Shift entities was borrowed from here: https://github.com/FWGS/hlsdk-xash3d/blob/blueshift/dlls/

Useful tutorials that helped me to get started:
https://www.moddb.com/games/half-life/tutorials/introduction-to-goldsrc-programming-setting-up-visual-studio
https://www.moddb.com/games/half-life/tutorials/where-is-poppy-your-first-custom-entity-part-1
https://www.moddb.com/games/half-life/tutorials/where-is-poppy-your-first-custom-entity-part-2

