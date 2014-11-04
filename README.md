![Fanatic Edition](http://i.imgur.com/cp3CFWO.png)

#####₪ About

<sub>Fanatic Edition is an experimental [Cube 2: Sauerbraten](http://sauerbraten.org) client modification, developed by [Fanatic](http://fanaticclan.com) members, available for Windows and Linux and [licensed](https://www.youtube.com/watch?v=IeTybKL1pM4) under the [zlib/libpng](http://www.opensource.org/licenses/zlib-license.php) license. The project itself is currently in *alpha* state and bugs/fixes appear daily. If you want to live on the bleeding edge of our development, feel free to try out our client and/or report bugs at our [IRC channel](https://kiwiirc.com/client/chat.kerat.net/?#fanatic).</sub>

#####₪ Features

<sub>★ Ingame IRC</sub><br />
<sub>★ Lua scripting</sub><br />
<sub>★ Image importing</sub><br />
<sub>★ Object importing</sub><br />
<sub>★ Ingame file sharing</sub><br />
<sub>★ Client side edit muting</sub><br />
<sub>★ Extended editing functions</sub><br />
<sub>★ Send your maps inclusive lightmaps</sub><br />
<sub>★ New special effects, playermodels and weapon enhancements</sub><br />

<sub>... and much more!</sub>

#####₪ Installation

<sub>Step §1:</sub><br />
<sub>Download the latest Cube 2: Sauerbraten SVN repository [here](http://svn.code.sf.net/p/sauerbraten/code) or [here](http://sourceforge.net/p/sauerbraten/code/HEAD/tree/).</sub>

<sub>Step §2:</sub><br />
<sub>Download the latest Fanatic Edition repository [here](https://github.com/fanaticclan/client/archive/master.zip).</sub>

<sub>Step §3:</sub><br />
<sub>Unzip and copy everythig into your fresh created Sauerbraten SVN directory.</sub>

<sub>Step §4:</sub><br />
<sub>If you dont want to compile yourself, get the latest binary for your system [here](https://github.com/fanaticclan/client/releases/latest); on linux, place *faned_client* into your *bin_unix/* directory, on windows, place *faned_client.exe* into your *bin/* directory.</sub>

<sub>Step §5:</sub><br />
<sub>On linux, run *faned.sh*, on windows, run *faned.bat*, to fire up the game.</sub>

#####₪ Updating

<sub>Step §1:</sub><br />
<sub>Download the latest Fanatic Edition repository [here](https://github.com/fanaticclan/client/archive/master.zip).</sub>

<sub>Step §2:</sub><br />
<sub>If you dont want to compile yourself, get the latest binary for your system [here](https://github.com/fanaticclan/client/releases/latest); on linux, place *faned_client* into your *bin_unix/* directory, on windows, place *faned_client.exe* into your *bin/* directory.</sub>

<sub>Step §3:</sub><br />
<sub>Delete your configuration file *faned_config.cfg*.</sub>

<sub>Step §4:</sub><br />
<sub>On linux, run *faned.sh*, on windows, run *faned.bat*, to fire up the game.</sub>

#####₪ Usage

<sub>**Filesharing**: The Fanatic Edition *sendfile* function allows you to send and share - for example - a *mymap.cfg* for your map *mymap.ogz*. Run the command `sendfile packages/base/mymap.cfg`; your config file will now be sent to the server. After that, your friend needs to run the command `getfile packages/base/mymap.cfg` to save it and `exec packages/base/mymap.cfg` to execute the config file. *sendfile* is not limited to map or configs files; you can share everything you want with it as long as the server is supporting it's file size. Other players without Fanatic Edition who may try to get your file using the default `getmap` command will get the default *could not read map* error instead of your file. Remember: This works only if both players are using Fanatic Edition or another modified client with a simliar *getfile* function ([SJM](http://sjm.altervista.org), [CMEd](http://sourceforge.net/projects/cmsauerbraten/)).</sub>

<sub>**Filesystem**: Fanatic Edition is made to run next to your vanilla Sauerbraten SVN client and not to interfer with its file system; if you want to use your personal auth.cfg or autoexec.cfg with Fanatic Edition, you need to rename them to *faned_auth.cfg* / *faned_autoexec.cfg*, otherwise Fanatic Edition will ignore these files.</sub>

<sub>**Importing Images**: With Fanatic Edition, you can easy import image files to your map using LUA. Place *myimage.png* into your sauerbraten main directory, ingame enter the following command: `lua importimagevcolor('myImage.png',100,100)` and wait till the converting process is finished. The full function's syntax is  `importimagevcolor('FiLE',WiDTH,HEIGHT,R,G,B,ALPHA)`.</sub>

<sub>**Importing Objects**: To use the blender importer, first install [Wrack's SMC Exporter](https://github.com/wrack/blender-smc-exporter) *io_export_smc.py*, located at *faned/tools/* into Blender: *File > User Preferences > Addons > Install Addon* and don't forget to activate it. Now create some geometry, select it, hit *File > Export > Sauerbraten (.smc)*, set desired grid and cubecount values (grid 0 and cubecount 50 are good values for testing) and back at Fanatic Edition, simply enter `importsmc PATH/FiLE.smc`. This will load the .smc (Sauerbraten Marching Cubes) file to your current selection.</sub>

<sub>**Ingame IRC**: To use the IRC function, simply go to *Main Menu > IRC*, connect to a server, join a channel and hit the new keybind »i« to chat. You can use default IRC commands like `NAMES`, `PRIVMSG`, `WHOIS` and `WHOWAS` within this keybind, too.</sub>

<sub>**Typewriting**: To use the typewriter, simply select a cube's *right* face and run the command `typegen "HELLO WORLD!"`. Note to write your text in uppercase (future fonts will also include lowercase characters). To change the typewriter's font, run the command `typegen_font "Directory"`. Currently included *public domain* fonts:</sub>

<sub>*»#yolo by Snowy« - Directory: tf_snowy_#yolo/*</sub><br />
<sub>*»1 by restcoser« - Directory: tf_restcoser_1/*</sub><br />
<sub>*»alphabet by TristamK« - Directory: tf_tristamk_alphabet/*</sub><br />
<sub>*»ANDROMEDA by Redon« - Directory: tf_redon_andromeda/*</sub><br />
<sub>*»BAD FONT by bum« - Directory: tf_bum_bad_font/*</sub><br />
<sub>*»Fanatic by Neo« - Directory: tf_neo_fanatic/*</sub><br />
<sub>*»GENERIC by TimmytheWhale« - Directory: tf_timmythewhale_generic/*</sub><br />
<sub>*»lib_rus by TristamK« - Directory: tf_tristamk_lib_rus/*</sub><br />
<sub>*»Pixel Font by Nyne« - Directory: tf_nyne_pixel_font/*</sub><br />
<sub>*»sYNDABC by sYNDERF« - Directory: tf_synderf_syndabc/*</sub><br />
<sub>*»Tiny Font by SomeDude« - Directory: tf_somedude_tiny_font/*</sub><br />

#####₪ Commands: Main

<sub>`abort` Use this for proper crashing FanEd without saving changes to config files;</sub><br />
<sub>`anticheat 0|1` Enable or disable the anticheat detection system;</sub><br />
<sub>`autorespawn 0|1`</sub><br />
<sub>`autosaydisconnect 0|1`</sub><br />
<sub>`autosaydisconnectmsg "STR"`</sub><br />
<sub>`autosaygg 0|1`</sub><br />
<sub>`autosayggmsg "STR"`</sub><br />
<sub>`autosaynp 0|1`</sub><br />
<sub>`autosaynpmsg "STR %s"` %s will be replaced with the player's name who teamkilled you;</sub><br />
<sub>`autosaysorry 0|1`</sub><br />
<sub>`autosaysorrymsg "STR %s"` %s will be replaced with the player's name you teamkilled;</sub><br />
<sub>`autosayteam 0|1`</sub><br />
<sub>`autoswitchclientteam CN` / `autounswitchclientteam CN`</sub><br />
<sub>`autoswitchweapon 0|1` Enable or disable automatic weapon switch on ammo pickup;</sub><br />
<sub>`backgroundbluremphasis 0-100`</sub><br />
<sub>`backgroundblurrepeat 0-100`</sub><br />
<sub>`backgroundfog 0.0-1.0`</sub><br />
<sub>`backgroundimagefile STR`</sub><br />
<sub>`backgroundlogofile STR`</sub><br />
<sub>`bloodcolour RGB|HEX`</sub><br />
<sub>`bloodintensity FLOAT`</sub><br />
<sub>`centerconsole 0|1`</sub><br />
<sub>`centerconsolealpha 0-255`</sub><br />
<sub>`centerconsoledelay 0-10000`</sub><br />
<sub>`cgshotcolor RGB|HEX`</sub><br />
<sub>`cgshotduration iNT`</sub><br />
<sub>`cgshotsize FLOAT`</sub><br />
<sub>`codepad` Helper for CubeScript/Lua ingame scripting;</sub><br />
<sub>`compass 0|1`</sub><br />
<sub>`confix iNT` Set fixed width for console lines;</sub><br />
<sub>`consmoothfade 0|1` Enable or disable smooth console fading;</sub><br />
<sub>`contimestamp 0|1|2` Enable or disable console time, 1: *fullconsole*, 2: *always*;</sub><br />
<sub>`crosshairalpha 0.0f-1.1f`</sub><br />
<sub>`crosshairbumpfx 0|1` Enable or disable the crosshair bump effect;</sub><br />
<sub>`crosshaircolor RGB|HEX` Change the color of your crosshair;</sub><br />
<sub>`crosshairnames 0|1` Enable or disable drawing info text next to the crosshair (player->name);</sub><br />
<sub>`crosshairnamesalpha 0-255`</sub><br />
<sub>`crosshairnamescolor 0-9-A-Z`</sub><br />
<sub>`crosshairnamesdelay 0-10000`</sub><br />
<sub>`crouch` Bind it to a key for experimental crouching (Note: Other players will **not** see you crouched.);</sub><br />
<sub>`ctfsoundset "STR"` Set the desired soundset for ctf modes;</sub><br />
<sub>`damagemotion 0|1` Enable or disable special effects after getting damage from explosives;</sub><br />
<sub>`damageparticle 0|1` Enable or disable the damage information particle above players heads;</sub><br />
<sub>`date %FORMAT` Returns the current date/time in given format; example: `echo (date %Y-%m-%d %H:%M:%S)`;</sub><br />
<sub>`deathcamera 0|1` Enable or disable experimental actor following after death (Note: Other players will **not** see you spectating.);</sub><br />
<sub>`deathpanicscreen 0|1` Enable or disable the deathpanic eyeveins screen;</sub><br />
<sub>`deathpanicsound 0|1` Enable or disable the deathpanic heartbeat sound;</sub><br />
<sub>`getbuildname` Returns "Cube 2: Sauerbraten - Fanatic Edition";</sub><br />
<sub>`getbuildrevision` Returns the current Cube 2: Sauerbraten SVN revision Fanatic Edition is based on;</sub><br />
<sub>`getbuildurl` Returns Fanatic Edition's current github URL;</sub><br />
<sub>`getbuildversion` Returns Fanatic Edition's current build version;</sub><br />
<sub>`getconline` Get the last console output; for specific words use `(at $getconline iNT)`;</sub><br />
<sub>`guialpha 0-255`</sub><br />
<sub>`guncolorfrags 0|1` Enable or disabled weapon related (colored) frag messages;</sub><br />
<sub>`highlight "STR"` Highlight STR (play highlight sound), default: *player1->name*;</sub><br />
<sub>`hudgunalpha 0-255`</sub><br />
<sub>`hudminiscoreboard 0|1` Show a small mini scoreboard at the right side of the hud;</sub><br />
<sub>`hudminiscoreboardalpha 0-255`</sub><br />
<sub>`hudminiscoreboardcolor 0-9-A-Z`</sub><br />
<sub>`hudminiscoreboardlimitcmode iNT` Limit the players shown at the mini scoreboard in ctf/collect mode (with radar);</sub><br />
<sub>`hudminiscoreboardlimit iNT` Limit the players shown at the mini scoreboard;</sub><br />
<sub>`hudstats 0|1`</sub><br />
<sub>`hudstatsalpha 0-255`</sub><br />
<sub>`hudstatscolor 0-9-A-Z`</sub><br />
<sub>`lasercolorrainbow 0|1`</sub><br />
<sub>`lasercolor RGB|HEX`</sub><br />
<sub>`lasercolorteam 0|1`</sub><br />
<sub>`lightningcolorrainbow 0|1`</sub><br />
<sub>`lightningcolor RGB|HEX`</sub><br />
<sub>`lightningcolorteam 0|1`</sub><br />
<sub>`pishotcolor RGB|HEX`</sub><br />
<sub>`pishotduration iNT`</sub><br />
<sub>`pishotsize FLOAT`</sub><br />
<sub>`playchatsound 0|1`</sub><br />
<sub>`playnearmisssound 0|1`</sub><br />
<sub>`playrespawnsound 0|1`</sub><br />
<sub>`positionpacketdelay 0-33` Less values are smoothing network lags;</sub><br />
<sub>`radar STR` Set custom radar texture;</sub><br />
<sub>`radarteammatesdead 0|1` Enable or disable radar icons for dead players (own team);</sub><br />
<sub>`radarteammatesplayerstarts 0|1` Enable or disable radar icons for playerstarts (own team);</sub><br />
<sub>`riflareduration iNT`</sub><br />
<sub>`riflaresize FLOAT`</sub><br />
<sub>`riflarecolor RGB|HEX`</sub><br />
<sub>`rishotduration iNT`</sub><br />
<sub>`rishotgravity iNT`</sub><br />
<sub>`rishotsize FLOAT`</sub><br />
<sub>`ritraillaser 0|1`</sub><br />
<sub>`ritraillightning 0|1`</sub><br />
<sub>`ritrailsmoke 0|1`</sub><br />
<sub>`ritrailspin 0|1`</sub><br />
<sub>`serverlistmax iNT` Set amount of servers shown at one server list page;</sub><br />
<sub>`setfont FONT` Hook for dynamic font changing; myFont.cfg needs to be executed before: (`exec packages/fonts/myFont.cfg`);</sub><br />
<sub>`sgshotcolor RGB|HEX`</sub><br />
<sub>`sgshotduration iNT`</sub><br />
<sub>`sgshotsize FLOAT`</sub><br />
<sub>`shotsparks 0|1`</sub><br />
<sub>`showbuildversion 0|1` Enable or disable Fanatic Edition's build version in main menu;</sub><br />
<sub>`showteamscores 0|1` Show team scores time at upper right corner/below minimap;</sub><br />
<sub>`showtimeremaining 0|1` Show remaining time at upper right corner/below minimap;</sub><br />
<sub>`smokecolorrainbow 0|1`</sub><br />
<sub>`smokecolor RGB|HEX`</sub><br />
<sub>`smokecolorteam 0|1`</sub><br />
<sub>`spincolorrainbow 0|1`</sub><br />
<sub>`spincolor RGB|HEX`</sub><br />
<sub>`spincolorteam 0|1`</sub><br />
<sub>`system STR` Runs any system command ingame and returns exit/error informations;</sub><br />
<sub>`teamblue` Join the blue team;</sub><br />
<sub>`teamred` Join the red team;</sub><br />
<sub>`thirdpersonalpha 0-100`</sub><br />
<sub>`zoomscope 0|1`</sub><br />

#####₪ Commands: Editing

<sub>`ambientocclusion 0-255`</sub><br />
<sub>`ambientocclusionradius 0.0-40.0`</sub><br />
<sub>`ambientocclusionprecision 0.0-17.0f`</sub><br />
<sub>`buildcylinder RADiUS HEiGHT POWER POSiTiON-X POSiTiON-Y POSiTiON-Z`</sub><br />
<sub>`buildhelix RADiUS HEiGHT POWER`</sub><br />
<sub>`buildmandelbrot` Customize via `mandelbrotgrid`, `mandelbrotwidth`, `mandelbrotheight`, `mandelbrotdepth` and `mandelbrotiterations`;</sub><br />
<sub>`buildoctahedron RADiUS POWER POSiTiON-X POSiTiON-Y POSiTiON-Z`</sub><br />
<sub>`buildparaboloid RADiUS POWER POSiTiON-X POSiTiON-Y POSiTiON-Z`</sub><br />
<sub>`buildsimplexnoise RADiUS POWER POSiTiON-X POSiTiON-Y POSiTiON-Z`</sub><br />
<sub>`buildsimplexsphere iNNER OUTER GRiD NOiSE SCALiNG THRESHOLD`</sub><br />
<sub>`buildsphere RADiUS POWER POSiTiON-X POSiTiON-Y POSiTiON-Z SHAPE-X SHAPE-Y SHAPE-Z)`</sub><br />
<sub>`buildtorus RADiUS RANGE DiRECTION POSiTiON-X POSiTiON-Y POSiTiON-Z`</sub><br />
<sub>`calclightradius RADiUS` Local calclight at player's current position with given radius; set to 0 to disable);</sub><br />
<sub>`clearbots` Remove all connected bots at once;</sub><br />
<sub>`clearpickups` Remove all ffa ents: ammo, health, armor, etc.;</sub><br />
<sub>`collisions 0|1` Enable or disbale collisions with other players or mapmodels;</sub><br />
<sub>`conoutf_healthboost 0|1` Enable or disable console output on health boost;</sub><br />
<sub>`conoutf_quaddamage 0|1` Enable or disable console output on quad damage;</sub><br />
<sub>`cubelimit iNT` Set a limit of cubes other players are allowed to change at once; warn when this limit exceeds;</sub><br />
<sub>`derpycn CN TYPE ATTR1 ATTR2 ATTR3 ATTR4 ATTR5`</sub><br />
<sub>`editent iNDEX X Y Z TYPE ATTR1 ATTR2 ATTR3 ATTR4 ATTR5` Entity scripting hook;</sub><br />
<sub>`editmute CN` / `uneditmute CN`</sub><br />
<sub>`fragrecstart` Start recording frags (actor, victim, location) to >fragrec.log;</sub><br />
<sub>`fragrecstop` Stop recording frags (actor, victim, location) to >fragrec.log;</sub><br />
<sub>`genmaze X Y SEED` Maze generation; grid is set by selection;</sub><br />
<sub>`gotodist iNT` Customize the distance for /goto and /gotosel;</sub><br />
<sub>`grid2Dselectioncolor RGB|HEX` Set the color of the 2D selection box in editmode;</sub><br />
<sub>`grid3Dselectioncolor RGB|HEX` Set the color of the 3D selection box in editmode;</sub><br />
<sub>`gridhoverselectioncolor RGB|HEX` Set the color of the hovered selection in editmode;</sub><br />
<sub>`gridselectioncolor RGB|HEX` Set the color of the selection in editmode;</sub><br />
<sub>`importrec FiLE` Import .rec files exported from CMEd;</sub><br />
<sub>`importsmc FiLE` Import .smc files exported from Blender;</sub><br />
<sub>`playasound iNT`</sub><br />
<sub>`playasoundat iNT CN`</sub><br />
<sub>`printsvarescaped 0|1` If enabled, re-print map SVAR (maptitle, cloudlayer, ...) changes escaped;</sub><br />
<sub>`rave 0|1` Just for fun: /rave 1 will loop through near all possible map variable colors, /rave 0 sets everything back like it was before;</sub><br />
<sub>`replacetex iNT iNT 0|1` Replace specific texture (iD) with another specific texture (iD), 0|1 everywhere or in selection only;</sub><br />
<sub>`receiveai 0|1` Enable or disable receiving initial packages for artifical intelligences aka bots;</sub><br />
<sub>`receivenewmap 0|1` Enable or disable receiving /newmap packets (/getmap to be back in sync);</sub><br />
<sub>`receiveremip 0|1` Enable or disable /remip packets (/getmap to be back in sync);</sub><br />
<sub>`receivesendmap 0|1` Enable or disable forced /getmap (#sendto by other players); Warning: This will disable your own /getmap too!;</sub><br />
<sub>`resetmonsters` Call this to update the monster entities after changing the spguard setting;</sub><br />
<sub>`resetmovables` Call this to update the movable entities after changing the spguard setting;</sub><br />
<sub>`savebak 0|1|2|3` backup saving; 0: *no backup*, 1: *mapname.BAK*, 2: *mapname_totalmillis.BAK*, 3: *mapname_date_time.BAK*;</sub><br />
<sub>`selectionguard 0|1` Enable or disable warnings about editing a selection that is not in view;</sub><br />
<sub>`setcampos X Y Z` Works only in coop edit mode;</sub><br />
<sub>`settex iNT`</sub><br />
<sub>`spguard 0|1` Enable or disable monsters, barrels, boxes, elevators and platforms (works online only for FanEd and CMEd users);</sub><br />
<sub>`teleport` Teleports you instantly to the location your crosshair is pointing at (default bind: "Q"); works only in coop edit;</sub><br />
<sub>`teleportcn CN X Y Z` Teleport CN from his current position to X Y Z;</sub><br />
<sub>`typegen "TEXT"` Write TEXT using premade cube fonts;</sub><br />
<sub>`typegen_font "DiRECTORY"` Change the typegen's font;</sub><br />

#####₪ Commands: Events

<sub>> `on* [ action ]` Additional event hooks for player1's CubeScript usage;</sub><br />

<sub>`onattack [ ]`;</sub><br />
<sub>`onclaim [ ]`;</sub><br />
<sub>`oncrouch [ ]`;</sub><br />
<sub>`ondamage [ ]`;</sub><br />
<sub>`ondeathbyenemy [ ]`;</sub><br />
<sub>`ondeathbyteammate [ ]`;</sub><br />
<sub>`ondropflag [ ]`;</sub><br />
<sub>`onflagfail [ ]`;</sub><br />
<sub>`onflagscore [ ]`;</sub><br />
<sub>`onfragenemy [ ]`;</sub><br />
<sub>`onfragteammate [ ]`;</sub><br />
<sub>`onjump [ ]`;</sub><br />
<sub>`onmapstart [ ]`;</sub><br />
<sub>`onresetflag [ ]`;</sub><br />
<sub>`onrespawn [ ]`;</sub><br />
<sub>`onreturnflag [ ]`;</sub><br />
<sub>`ontakeflagenemy [ ]`;</sub><br />
<sub>`ontakeflagteam [ ]`;</sub><br />
<sub>`ontaunt [ ]`;</sub><br />

#####₪ Commands: History

<sub>> Note: These commands load and save your /command history manually;</sub><br />
<sub>> By default, FanEd will load your /command history automatically;</sub><br />

<sub>`loadhistory`;</sub><br />
<sub>`savehistory`;</sub><br />

#####₪ Commands: Informations

<sub>> `$get*` Additional client informations (own client);</sub><br />

<sub>`$getarmour`;</sub><br />
<sub>`$getcamposx`;</sub><br />
<sub>`$getcamposy`;</sub><br />
<sub>`$getcamposz`;</sub><br />
<sub>`$getgunselect`;</sub><br />
<sub>`$gethealth`;</sub><br />
<sub>`$getphysstate`;</sub><br />
<sub>`$getping`;</sub><br />
<sub>`$getpitch`;</sub><br />
<sub>`$getspeed`;</sub><br />
<sub>`$getstate`;</sub><br />
<sub>`$getsuicides`;</sub><br />
<sub>`$getteamkills`;</sub><br />
<sub>`$getvelx`;</sub><br />
<sub>`$getvely`;</sub><br />
<sub>`$getvelz`;</sub><br />
<sub>`$getyaw`;</sub><br />

<sub>> `(getclientinfo CN TYPE)` Get client informations (all clients);</sub><br />

<sub>`TYPE  1: return client's current ai`;</sub><br />
<sub>`TYPE  2: return client's current armour`;</sub><br />
<sub>`TYPE  3: return client's current armourtype`;</sub><br />
<sub>`TYPE  4: return client's current attackchan`;</sub><br />
<sub>`TYPE  5: return client's current attacksound`;</sub><br />
<sub>`TYPE  6: return client's current deaths`;</sub><br />
<sub>`TYPE  7: return client's current edit`;</sub><br />
<sub>`TYPE  8: return client's current flagpickup`;</sub><br />
<sub>`TYPE  9: return client's current flags`;</sub><br />
<sub>`TYPE 10: return client's current frags`;</sub><br />
<sub>`TYPE 11: return client's current gunselect`;</sub><br />
<sub>`TYPE 12: return client's current health`;</sub><br />
<sub>`TYPE 13: return client's current idlechan`;</sub><br />
<sub>`TYPE 14: return client's current idlesound`;</sub><br />
<sub>`TYPE 15: return client's current lastaction`;</sub><br />
<sub>`TYPE 16: return client's current lastattackgun`;</sub><br />
<sub>`TYPE 17: return client's current lastbase`;</sub><br />
<sub>`TYPE 18: return client's current lastnode`;</sub><br />
<sub>`TYPE 19: return client's current lastpain`;</sub><br />
<sub>`TYPE 20: return client's current lastpickup`;</sub><br />
<sub>`TYPE 21: return client's current lastpickupmillis`;</sub><br />
<sub>`TYPE 22: return client's current lastrepammo`;</sub><br />
<sub>`TYPE 23: return client's current lasttaunt`;</sub><br />
<sub>`TYPE 24: return client's current lastupdate`;</sub><br />
<sub>`TYPE 25: return client's current lifesequence`;</sub><br />
<sub>`TYPE 26: return client's current ownernum`;</sub><br />
<sub>`TYPE 27: return client's current o.x`;</sub><br />
<sub>`TYPE 28: return client's current o.y`;</sub><br />
<sub>`TYPE 29: return client's current o.z`;</sub><br />
<sub>`TYPE 30: return client's current physstate`;</sub><br />
<sub>`TYPE 31: return client's current ping`;</sub><br />
<sub>`TYPE 32: return client's current pitch`;</sub><br />
<sub>`TYPE 33: return client's current plag`;</sub><br />
<sub>`TYPE 34: return client's current playermodel`;</sub><br />
<sub>`TYPE 35: return client's current privilege`;</sub><br />
<sub>`TYPE 36: return client's current respawned`;</sub><br />
<sub>`TYPE 37: return client's current smoothmillis`;</sub><br />
<sub>`TYPE 38: return client's current state`;</sub><br />
<sub>`TYPE 39: return client's current suicided`;</sub><br />
<sub>`TYPE 40: return client's current tokens`;</sub><br />
<sub>`TYPE 41: return client's current totaldamage`;</sub><br />
<sub>`TYPE 42: return client's current totalshots`;</sub><br />
<sub>`TYPE 43: return client's current vel.magnitude()`;</sub><br />
<sub>`TYPE 44: return client's current vel.x`;</sub><br />
<sub>`TYPE 45: return client's current vel.y`;</sub><br />
<sub>`TYPE 46: return client's current vel.z`;</sub><br />
<sub>`TYPE 47: return client's current weight`;</sub><br />
<sub>`TYPE 48: return client's current yaw`;</sub><br />

<sub>> Additional:</sub><br />

<sub>`(getclientip iNT)`</sub><br />
<sub>`(getteamscore STR)`</sub><br />

#####₪ Commands: IRC

<sub>`ircaddchan`</sub><br />
<sub>`ircaddclient`</sub><br />
<sub>`ircaddrelay`</sub><br />
<sub>`ircauth`</sub><br />
<sub>`ircbind`</sub><br />
<sub>`ircconnect`</sub><br />
<sub>`ircconns`</sub><br />
<sub>`ircdisconnect`</sub><br />
<sub>`ircfilter`</sub><br />
<sub>`ircfriendlychan`</sub><br />
<sub>`irchighlight "STR"`</sub><br />
<sub>`ircjoinchan`</sub><br />
<sub>`ircnick`</sub><br />
<sub>`ircpass`</sub><br />
<sub>`ircpasschan`</sub><br />
<sub>`ircport`</sub><br />
<sub>`ircquitmessage "STR"`</sub><br />
<sub>`ircrelaychan`</sub><br />
<sub>`ircsay`</sub><br />
<sub>`ircserv`</sub><br />

#####₪ Commands: Physics

<sub>> Note: These commands work only in coop edit mode;</sub><br /></sub>

<sub>`airjump 0|1` Airjumping: (hit jump key while in air);</sub><br />
<sub>`bunnyhop 0|1` Bunny hopping: (hold jump key for continuous jumping);</sub><br />
<sub>`walljump 0|1` Walljumping: (hit jump key while jumping against a wall);</sub><br />
<sub>`walljumpvel FLOAT`</sub><br />

<sub>`floorz FLOAT`</sub><br />
<sub>`gravity FLOAT`</sub><br />
<sub>`jumpvel FLOAT`</sub><br />
<sub>`slopez FLOAT`</sub><br />
<sub>`stairheight FLOAT`</sub><br />
<sub>`velocity FLOAT`</sub><br />
<sub>`wallz FLOAT`</sub><br />

#####₪ Commands: View

<sub>`lookup iNT`</sub><br />
<sub>`lookdown iNT`</sub><br />
<sub>`lookleft iNT`</sub><br />
<sub>`lookright iNT`</sub><br />

#####₪ Commands: VSlots

<sub>> `(get* iNT)` Get vSlot informations: iNT = Texture-ID, STR = Shader Parameter;</sub><br />

<sub>`(getvalpha iNT)`;</sub><br />
<sub>`(getvcolor iNT)`;</sub><br />
<sub>`(getvlayer iNT)`;</sub><br />
<sub>`(getvoffset iNT)`;</sub><br />
<sub>`(getvrotate iNT)`;</sub><br />
<sub>`(getvscale iNT)`;</sub><br />
<sub>`(getvscroll iNT)`;</sub><br />
<sub>`(getvshaderparam iNT STR)`;</sub><br />

#####₪ Credits

<sub>★ **[Nyne](https://github.com/gitnyne/)**: Main Development, Design, C, Lua, CubeScript</sub><br />
<sub>★ **[bum](https://github.com/Incognito357)**: C, Lua, CubeScript</sub><br />
<sub>★ **[Pyccna](http://quadropolis.us/user/5493)**: Playermodel Reskins and Hudguns</sub><br />
<sub>★ **[Wrack](https://github.com/wrack/)**: Marching Cubes functions</sub><br />
<sub>★ **Q009**: SauerEnhanced functions</sub><br />
<sub>★ **[AC](http://sourceforge.net/u/ac-sauer/profile/)**: CMEd functions</sub><br />
<sub>★ **[RaZgRiZ](http://pastebin.com/u/RaZgRiZ)**: CubeScript Martial Arts</sub><br />

#####₪ Original Cube 2: Sauerbraten:

<sub>★ **[Wouter "Aardappel" van Oortmerssen](http://strlen.com/)**</sub><br />
<sub>★ **[Lee "eihrul" Salzman](http://sauerbraten.org/lee/)**</sub><br />
<sub>★ Mike "Gilt" Dysart</sub><br />
<sub>★ Robert "baby-rabbit" Pointon</sub><br />
<sub>★ John "geartrooper" Siar</sub><br />
<sub>★ Quinton "Quin" Reeves and others.</sub><br />

#####₪ Contact

<sub>**IRC**:     [chat.kerat.net #fanatic](https://kiwiirc.com/client/chat.kerat.net/?#fanatic)</sub><br />
<sub>**Server**:  192.95.30.66 28785</sub><br />
<sub>**Website**: [fanaticclan.com](http://fanaticclan.com)</sub><br />
