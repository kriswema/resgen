# RESGen #
Develop: [![Build Status](https://travis-ci.org/kriswema/resgen.png?branch=develop)](https://travis-ci.org/kriswema/resgen)

RESGen is a tool to create res (resource) files for Half-Life. If a Half-Life map has a corresponding res file, Half-Life is able to send all resources the resources the map uses to the clients, if they don't have them. This helps a great deal when running a server with custom maps, instead of the defaults. Most players do not have these custom maps and the resources that should go with them. The res file enables them to download the map via Half-Life and start playing right away.
The problem with res files, however, is that it can take hours to create one if the resources used by a map are not known. RESGen can shorten this time to mere seconds. It reads the maps BSP file and searches it for used resources. The results of this search are then used to create the res file.

## Usage ##
* -a [rfafile]
  * The contents of the rfa file will be added to the end of the res file. This is useful when adding custom resources, like the StatsMe sound pack. The .rfa file extension is optional.
* -b [rfafile]
  * Excludes generated resources listed in [rfafile] (Default exclude .rfa's included). Useful to avoid log spam and steam http download problems. It is recommended to use this feature for steam servers, but note that it causes a speed decrease generating the res files.
* -c
  * Displays the RESGen credits.
* -d [folder]
  * [folder] will be searched for bsp files. A trailing (back)slash is optional.
* -e [modpath]
  * Makes RESGen verify that all resources in the res file actually exist. Resources that can't be found will be excluded from the res file. RESGen expects modpath to point to a valid mod directory structure. If the modpath isn't the valve folder, RESGen will try to find the valve folder too, so a complete resource list can be established.
* -f [map]
  * A res file will be generated for [map]. The .bsp file extension is optional.
* -g
  * Displays content of generated .res files and reports all missing resources. Causes slower performance.
* -h
  * Displays the help screen.
* -i
  * Displays maps found while building the map list. Only useful with the -r and/or -d option.
* -j
  * Resources found while building the resource list will be displayed. Useful with -e option.
* -k
  * Windows only. RESGen will not wait for the user to press a key to exit.
* -l
  * Turns on converting all res file entries to lowercase. RESGen converts all res file entries to lowercase since this is the default for Half-Life files. It has to do this because a lot of resource files in maps don't have the proper case that matches the actual resources. Only use this option if you know what you are doing.
* -m
  * Matches the case of the res file entries to the actual case of the files on disk. This prevents any missing resources because of wrong case in filenames. Recommended on Linux. If this option is used, the -l option will have no effect. Only works if used with the -e option.
* -o
  * If a res file already exists it will be overwritten. Removes old res files if the new file doesn't contain any res entries and no file is specified with the -a option.
* -p
  * Prevents RESGen from using the contents of any pakfile for resource verification. Thus, any resource that is available, but in a pakfile is excluded from the res file. This option is only useful when the -e option is also used. Please note that if a map comes with it's own pakfile, using this option will generate a res file that is incomplete.
* -r [folder]
  * [folder] and it's subfolders will be searched for bsp files. A trailing (back)slash is optional.
* -s
  * RESGen will display it's status line. This might considerably slow down res file generation, especially on smaller maps.
* -t
  * Linux only. RESGen will ignore symbolic links when searching folders for .bsp files. Please note that this does NOT affect resource searching. Useful with -d or -r options.
* -u
  * Parses relevant resource files to make sure they are actually used (such as WAD files) or to check if they have dependencies (such as MDL files with seperate textures). Only works with -e option.
* -v
  * Makes RESGen only give minimal output. It's recommended you use this if you want to create res files as fast as possible. RESGen will still report any error.
* -w
  * Displays the warranty for RESGen.
* -x [map]
  * Exclude this map from res file generation. Only works on maps found with -d or -r options. The .bsp file extension is optional.

Example 1:

  `resgen -ok -r C:\sierra\half-life\tfc -e C:\sierra\half-life\tfc -a shadowlord.rfa`

This will make RESGen generate res files for all maps that are in the tfc folder. It will verify it's resources from the tfc folder and the valve folder (C:\sierra\half-life\valve). Any existing res file will be overwritten. The contents of shadowlord.rfa will be added to all res files. When RESGen is finished it will exit immediately, not waiting for a keypress.

Example 2:

  `resgen -o -d C:\Sierra\half-life\tfc\maps -a customsounds.rfa -b res_tfc.rfa`

This will make RESGen generate res files for all maps that are in the maps folder. Any existing res file will be overwritten. The contents of customsounds.rfa will be added to all res files. Resources from res_tfc.rfa (these include valve resource) will not be added to generated res files.

### Using resource verification ###
The -e option, can be quite powerful. It can be used several ways. Here I try to describe a few of the more common ways, although there are many more.

NOTE: There is a subtle difference between Win32 and the Linux when checking if a resource is available or not. On Win32 RESGen does not care about case when comparing the found files to the res entry. On Linux it does care about it and reject resources when the case doesn't match. In 99% of the cases this doesn't matter. However, because of this difference it it's recommended to run RESGen on the target platform whenever possible.


Method 1, the simple way:

This method is most commonly used by server admins that want to add res files to their servers. Run RESGen on the mod folder you want to make res files for. Point the -e option to this folder too. This will generate good res files, which contain all found resources. You should not use the -p option unless you are certain that you have no maps with their own pakfile.

Example: `resgen -r half-life\cstrike -e half-life\cstrike`


Method 2, the mapper way:

You run resgen on the map, and point the -e option to the folder with the resources (these can be the same). Please note that the resource folder must match the folder layout of a normal folder. Using -p is possible if you aren't using a pakfile for your map. Also, note that resgen will look for the map info txt and overviews relative to the map itself, not the folder specified with -e

Example: `resgen -f mymap.bsp -e mapping\resourcetank`

## Credits ##
This program was made by Jeroen "ShadowLord" Bogers with serveral improvements and additions by Zero3Cool.

Special thanks to:
* "HoundDawg" and UnitedAdmins for helping RESGen grow.
* "Zero3Cool" for improving RESGen and keeping it alive.
* TheZProject.org for hosting RESGen.
* "luvless" for having the great idea to create this app.
* Valve for creating Half-Life!
* Everyone who uses RESGen.

http://resgen.hltools.com

## License ##
Copyright (C) 2000-2005 Jeroen Bogers, Zero3Cool

Copyright (c) 2013 [GitHub contributors] (https://github.com/kriswema/resgen/contributors)

RESGen is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

RESGen is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RESGen; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

## Changelog ##
Version 2.0.2
 - Bugfix: Fixed flawed logic that caused RESGen not to parse skynames and
   in some cases of non-standard BSP styles, the wad files. Thanks go to
   jtp10181 for finding this bug and suggesting a fix.

Version 2.0.1
 - Bugfix: Fixed minor bug in VString class that could occur if an empty
   string was concatenated with an existing. No know instances of this have
   occured because RESGen uses several sanity checks before doing string
   operations.
 - Fix: RESGen only supported BSP files generated on Windows or Unix/Linux
   because of the way line endings are handeled. Support for BSP files
   generated on a Mac/Apple (or using Apple style newlines) has been added.
 - Note: Decided to release this version as 'final' version 2. Note that
   this doesn't exclude any future bugfixes. It's just to indicate the
   current maturity of RESGen 2.

Version 2.0 RC2
 - Bugfix: Fixed bug in exclude file parsing. When the parser encountered a
   comment it would consider every following line a comment as well because
   it failed to clear the line buffer.
 - Bugfix: Fixed bug with loading exclude files. Forgot to convert
   backslashes to slashes, which means that any exclude located in a folder
   would never be matched if the exclude file used Windows format.

Version 2.0 RC1
 - Fix: Made some improvements to memory cleanup at program termination.
   While this probably doesn't make any difference to program preformance,
   it does score pretty high on the 'proper coding' meter.
 - Merge: The changes made by Zero3Cool have been merged with BETA 3 to
   supply you with the very best of RESGen in 1 great package!
   Zero3Cool's changes are:
     - Added -g flag (Displays content of generated .res files. By default
       RESGen will now NOT show this by default (causes slower
       performance)).
     - Added -b [rfafile] flag (Exclude resources from [rfafile] from being
       added to generated .res files).
     - Changed -i flag to display found maps while searching folders.
     - Removed some verbality to speed things up.
 - Fix: To keep in 'spirit' with Zero3Cool's changes, the -s and -j
   switches have been altered to display extra output instead of preventing
   output to show up.
 - Fix: Removed some more verbality for -v mode. Output is now bare
   minimum.
 - Fix: Rewrote Zero3Cool's parsing code for excluding resources to be more
   reliable.
 - Fix: Rewrote list lookups (used with resource verification and resource
   exclusion). Due to the new lookup system parsing speed should increase a
   lot over previous versions if these options are used.
 - Fix: Fixed some VString bugs (RTrim was broken) and some other minor
   changes. Added extra compare function.
 - Upgrade: Upgraded LinkedList with some improvements made earlier, but
   which were not yet integrated with this version. Added sorting and find
   functions to sort the lists. The find function can rapidly find a node
   in the list, provided it's sorted. To extend the usefulnes of the
   LinkedList class and to further optimize sorting an InsertAt and
   InsertSorted function has been added.
 - Fix: Fixed some small errors in LinkedList and VString classes and added
   some functionality. If you are using these classes in another project it
   is recommended you upgrade them with these new versions.
 - New: Added case correction for res file entries with -m switch. For it
   to operate resource verification (-e) should be used. It uses the
   resource list built with resource verification to correct case.
   -m overrides -l.
 - Fix: In overwrite mode (-o) res files will be deleted if there are no
   resources found. This prevents old res files from staying behind with
   wrong entries. This is especially the case when using options like -b
   or -e.
 - Fix: Changed -l option to default NOT to convert entries to lowercase.
   You must now specify -l to have your entries converted to lowercase.
 - New: Added WAD file parsing. RESGen will now check if a WAD file
   actually contains textures used by the map. It also reports any missing
   textures. Use the -u switch to activate the checking.
 - New: Added MLD file parsing. RESGen will now check if a MDL file
   requires a seperate texture file. If it does, this texture file is added
   to the res file. Use the -u switch to activate the checking.
 - New: RESGen reports res files that have missing resource because they
   were not found (only applies to -u and -e options). Use -g to see which
   wad files resources are missing and which resources were excluded.

Version 2.0 BETA 3
 - Bugfix: Overviews were not properly added to the res file. RESGen
   only looked for a bmp file. To make matters worse, it added it as a tga
   file to the res file. This behaviour has been corrected. RESGen now
   looks for a tga and a bmp file (favouring the tga if both are present).
 - Fix: Half-Life already transfers the map's txt file to the client,
   without looking at the res file. RESGen used to add this txt file to
   the res file too. Since this is not needed and can even create errors,
   RESGen now does not add the map's txt file to the res file.

Version 2.0 BETA 2
 - Bugfix: The parsing enigne crashed when the BSP entity data wasn't in
   Linux format. It now can work with data in Windows format too. If the
   BSP is in any other format, the parser will generate an error, instead
   of crashing.
 - Bugfix: If there were no WAD files specified for a map, the parser gave
   an error message for the map and stopped parsing it. It now correctly
   handles this and can continue parsing the rest of the BSP.
 - Fix: Entity data corruption errors now give a bit more information about
   what is wrong with the data.
 - Bugfix: While running RESGen I discovered that the memory use increased
   with every parsed map. After running a memory leak detector and
   reviewing the code, I discovered that I forgot to close the BSP files
   after reading them (*doh*). They are now properly closed and there is no
   more memory leaking. (That I know of anyways :)

Version 2.0 BETA 1
 - New: Initial release. Total rework of the parsing engine. Also much more
   object oriented.

### Changes between v1 and v2 ###
From the outside RESGen 1 and 2 look similar. On the inside a lot of changes have been made. First of all, major parts have been rewritten, and other parts have had a major upgrade. The biggest change is that the RESGen core is fully C++ now, instead of a bit C++ in a C program. Because of this I have DROPPED the scripting return values. It's very easy to use the RESGen source in your own programs now.

The RESGen parser should be a bit faster now. It's speed mainly depends on the speed of your hard disk and your output window. That last limitation can be overcome by using the -v option (minimal output).

The last major change is the -e option. It enables you to have RESGen automatically verify if you server actually has the resources the map claims to need. This especially applies to missing wad files. See chapter 5 for more information on how to use the -e option.

Additionally, RESGen now tries to locate the maps information txt and overview pictures. Please note that RESGen can only find these if they are stored in the same folder layout as they would be on the server (mapname.txt, ../overviews/mapname.txt and ../overviews/mapname.bmp).
