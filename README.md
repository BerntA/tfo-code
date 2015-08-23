#The Forgotten Ones Source Code

##About
This was initially a total conversion mod for Half-Life 2, when Greenlight came around I decided to give it a try.
Eventually The Forgotten Ones got greenlit and I was probably as surprised as you reading this right now.
It was a major achievement for me, so here I'm years later releasing the source code of this long project.

The game takes place in Germany during the 60's, you play as a detective whose name is Grobuskna Vladinov.
His family suffered deep tragedies during the holocaust and he has been on the hunt for the person(s) responsible ever since.

##Current State
The game has been on Steam for over a year now, the code is quite stable. 
However unused stuff such as the Custom Story code might be a bit unpolished. 
I have also compiled this on Linux Ubuntu and Mac OSx Yosemite without any issues.

##Navigating
All the game code is under:
- game/client/
- game/server/
- game/shared/

For only The Forgotten Ones stuff:
- game/client/tfo/
- game/server/tfo/
- game/shared/tfo/

I've edited most of the original SDK 2013 files so there's edits pretty much everywhere. 
You'll find the most advanced features in the tfo folders tho. Like the inventory stuff.

##Third-Party Libraries
I'm using the FMOD API to allow music during level transitions and in the main menu and an updated version of Steam API.

##Contributing
You're free to make pull requests for bug fixes.

##Features
- A custom GameUI and VGUI structure
- Loading Music
- Custom HUD
- Inventory System + 3D rendered items which you can inspect
- Dialogue System
- Bleeding System
- Custom loading screen which reports the actual loading progress
- Custom NPCs
- Custom Weapons
- Blood rendering on viewmodel when taking damage or if blood splats on you
- Note System
- Achievement & Stat handler
- New VGUI Controls
- Custom Console
- Slot Saving System
- Boss Health Bar
- Thirdperson animations and thirdperson support in general
- The player can be rendered in mirrors while in firstperson
- Radius based glow outline effect which helps the player find important items
- Film Grain
- Dynamic Scope Support
- Ironsights
- Improved vehicles
- Scriptable NPCs
- High Modding Support (including the unused Custom Story code which was removed in V2.9)
- Cross-Platform friendly code
- And much more...

##Documentation
Most of the documentation provided is examples on how to link the scripts with the Hammer World Editor & GUI, for example if you want to add a new inventory item. 

##Crediting
I'd appreciate if you'd credit me if you want to use my work.

##Disclaimer
This project started in 2011, I was a rookie at the time so there might be some untidy code here and there.

For V2.9 I refactored and went through all of my code to polish it as good as I could, some parts are still a bit hard coded but it'll do for now. 

###Have fun and good luck breaking code.
