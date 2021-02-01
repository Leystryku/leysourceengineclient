# LeySourceEngineClient
[![License](https://img.shields.io/badge/license-MIT-green)](https://opensource.org/licenses/MIT)
[![App Version](https://img.shields.io/badge/version-v2.0.0-brightgreen)](https://github.com/Leystryku/mpbomberman_racket)

A small Source Engine Client which deals with the basic communication neccessary to connect to a Source Engine Server which is based on a OB Game (e.g. Garry's Mod, TF2, etc.). If you do use this or use from code from this PLEASE give credit. Ever since this has been released several people have created their "own version" while not giving any credits to the original. Please don't be that guy. If I release code open source, you are free to use it as much as you like. You do not have to pay for it whatsoever. But please give credit when using my work or using code from my work.

# Running
- Create steam_appid.txt in the same directory and write the AppID 4000 inside
- Just run leysourceengineclient.exe -serverip ip -serverport port -nickname nick

#### Troubleshooting
If it crashes with a MessageBox on startup install the VC++ 2019 redists from microsoft https://www.microsoft.com/.


# Compiling
If you wish to compile this yourself:
- Run vs2019.bat
- Open project/leysourceengineclient.sln
- Compile it using MSVC/MSVS

The only supported platform right now is Windows. Feel free to contribute!

![In program view](https://raw.githubusercontent.com/Leystryku/leysourceengineclient/master/assets/preview.jpg)



# Credits
- Leystryku (me)
	* Creating this project
    * Reverse engineering the code required to make it work
- Valve
  * Creating the source engine and steamworks stuff
