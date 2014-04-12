/*
RESGen. A tool to create .res files for Half-Life.
Copyright (C) 2000-2005 Jeroen Bogers

This file is part of RESGen.

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
*/

/* Command line parameters:
-h help
-c credits
-w warranty

-v non-verbal mode
-s display status line
-i display found maps while generating map list
-j display found resources while generating resource list
-g display contents of generated .res files and missing resources

-d [folder] entire folder
-r [folder] entire folder + subfolders
-f [map] parse map
[map] parse map (same as -f)
-x [map] exclude map from res file generation (use with -d or -r)

-o overwrite existing res files
-a [rfafile] add contents of [rfafile] to res file(s)

-l do not convert res entries to lowercase
-m matches file case with resources found by -e
-e [modpath] check for resource existance
-p do not check for resource existance in pakfiles
-u parses wads for used textures and mdls for external textures (use with -e)
-b [rfafile] Excludes files from [rfafile]

-k *WIN32* do not wait for keypress before exit
-t *LINUX* ignore symbolic links when searching directories

// v1 -> v2 command line changes:
-w -> [CHANGE] Win32 exit mode to warranty
-i -> [OS] Not Linux specific anymore
-u -> [REMOVED] Function is prone to error
-k -> [NEW] Replacement for old -w
-e -> [NEW] Requested function.
-p -> [NEW] To go with -e
-x -> [NEW] Good for advanced users

// v2B3 -> v2RC1 command line changes:
-b -> [NEW] Excludes files from [rfafile]
-i -> [CHANGE] Now displays maps while generating map list (folders will never be displayed anymore)
-g -> [NEW] Display content of generated .res files
-m -> [NEW] matches file case with resources found by -e
-u -> [NEW] parses wads for used textures and mdls for external textures (use with -e)

-n do not ignore unused wads (use with -u)

// Param usage
abcdefghijklmnopqrstuvwxyz
xxxxxxxxxxxxx xx xxxxxxx
// Free: q y z
*/

// if you define NO_MULTIARG_FILES RESGen will reject any multiarg entries for:
// d, r, f, x, a, b and e
#define NO_MULTIARG_FILES

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef WIN32
#include <conio.h>
#include <windows.h>
#endif

#include "listbuilder.h"
#include "resgenclass.h"
#include "resgen.h"
#include "leakcheck.h"

#ifdef WIN32
void getexitkey(bool verbal, bool keypress)
{
	if (verbal && keypress)
	{
		printf("Press any key to exit");
		_getch();
	}
}
#endif

// help & copyrights
void showcopyright()
{
	printf("RESGen version %s, Copyright (C) 2000-2005 Jeroen Bogers and Zero3Cool\n", VERSION);
	printf("RESGen comes with ABSOLUTELY NO WARRANTY; for details\n");
	printf("use the command line switch '-w'.  This is free software,\n");
	printf("and you are welcome to redistribute it under certain\n");
	printf("conditions; see the 'gpl.txt' file for details.\n\n");
}

void showhelp()
{
//		Win32 console size
//	       0         1         2         3         4         5         6         7         8
//	        12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("Command line parameters:\n");
	printf(" -h           Displays this help screen\n");
	printf(" -c           Displays credits\n");
	printf(" -w           Show the extended copyright screen\n");

	printf(" -v           Puts RESGen in silent mode (little console output)\n");
	printf(" -s           Display the status line\n");
	printf(" -i           Display found maps while generating map list\n");
	printf(" -j           Display found resources while generating resource list\n");
	printf(" -g           Display contents of generated .res files and missing resources\n");

	printf(" -d [folder]  Process entire folder\n");
	printf(" -r [folder]  Same as -d, but processes subfolders too\n");
	printf(" -f [map]     Process this map\n");
	printf(" [map]        Same as -f\n");
	printf(" -x [map]     Exclude map from res file generation (use with -d or -r)\n");

	printf(" -o           Overwrite existing res files\n");
	printf(" -a [rfafile] Adds the rfa file to generated res files\n");

	printf(" -l           Convert res entries to lowercase\n");
	printf(" -m           Match the file case with the resources on disk (use with -e)\n");
	printf(" -e [modpath] Check for resource existance\n");
	printf(" -p           Do not check for resource existance in pakfiles\n");
	printf(" -u           Parse WAD and MDL files for external textures (use with -e)\n");
	printf(" -n           Do not ignore unused WAD files (use with -u)\n");

	printf(" -b [rfafile] Excludes resources from [rfafile] from generated res files.\n");

	#ifdef WIN32
	printf(" -k           RESGen will not wait for a keypress to exit in verbal mode\n");
	#else
	printf(" -t           Ignore symbolic links when searching folders\n");
	#endif

	printf("\nExample:\n");
	printf("   resgen -f boot_camp -d . -r ../mappack -e hlds_l/cstrike\n\n");

	printf("Read RESGen.txt for more information!\n");
}

void showwarranty()
{
	printf("RESGen. A tool to create .res files for Half-Life.\n");
	printf("Copyright (C) 2000-2005 Jeroen Bogers and Zero3Cool\n\n");

	printf("RESGen is free software; you can redistribute it and/or modify\n");
	printf("it under the terms of the GNU General Public License as published by\n");
	printf("the Free Software Foundation; either version 2 of the License, or\n");
	printf("(at your option) any later version.\n\n");

	printf("RESGen is distributed in the hope that it will be useful,\n");
	printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	printf("GNU General Public License for more details.\n\n");

	printf("You should have received a copy of the GNU General Public License\n");
	printf("along with RESGen; if not, write to the Free Software\n");
	printf("Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n");
}

void showcredits()
{
	printf("This program was made by Jeroen \"ShadowLord\" Bogers,\n");
	printf("with serveral improvements and additions by Zero3Cool.\n\n");

	printf("Special thanks to:\n");
	printf("  \"HoundDawg\" and UnitedAdmins for helping RESGen grow.\n");
	printf("  \"Zero3Cool\" for improving RESGen and keeping it alive.\n");
	printf("  TheZProject.org for hosting RESGen.\n");
	printf("  \"luvless\" for having the great idea to create this app.\n");
	printf("  Valve for creating Half-Life!\n");
	printf("  Everyone who uses RESGen.\n\n");

	printf("http://resgen.hltools.com\n");
}

int main(int argc, char* argv[])
{
	// init config struct
	config_s config;

	config.help = false;
	config.credits = false;
	config.warranty = false;

	config.verbal = true;
	config.statusline = false;
	config.searchdisp = false;
	config.resourcedisp = false;
	config.contentdisp = false;

	config.overwrite = false;

	config.tolower = false;
	config.matchcase = false;
	config.checkpak = true;
	config.parseresource = false;
	config.preservewads = false;

#ifdef WIN32
	config.keypress = true;
#else
	config.symlink = true;
#endif

	// parse the command line
	for (int i = 1; i < argc; i++) // arg 0 is the command line.
	{
		char *argstr = argv[i];
		if (argstr[0] == '-')
		{
			// cmdline switch(es)
			int arglen = strlen(argstr);
			for (int j = 1; j < arglen; j++)
			{
				switch (argstr[j])
				{
// -h
				case 'h':
					config.help = true;
					break;
// -c
				case 'c':
					config.credits = true;
					break;
// -w
				case 'w':
					config.warranty = true;
					break;
// -v
				case 'v':
					config.verbal = false;
					break;
// -s
				case 's':
					config.statusline = true;
					break;
// -i
				case 'i':
					config.searchdisp = true;
					break;
// -j
				case 'j':
					config.resourcedisp = true;
					break;
// -g
				case 'g':
					config.contentdisp = true;
					break;
// -d
				case 'd':
				{
#ifdef NO_MULTIARG_FILES
					if (arglen != 2)
					{
						printf ("Ignoring 'd' argument: Cannot be used in multiple argument list\n");
						break;
					}
#endif
					if (i == argc - 1)
					{
						printf ("Ignoring 'd' argument: No folder specified\n");
						break;
					}
					if (argv[i+1][0] == '-')
					{
						printf ("Ignoring 'd' argument: No folder specified\n");
						break;
					}

					file_s *file = new file_s;
					file->folder = true;
					file->recursive = false;
					i++; // increase i.. we used that arg.
					file->name = argv[i];

					config.files.AddTail(file); // add to linked list
					break;
				}
// -r
				case 'r':
				{
#ifdef NO_MULTIARG_FILES
					if (arglen != 2)
					{
						printf ("Ignoring 'r' argument: Cannot be used in multiple argument list\n");
						break;
					}
#endif
					if (i == argc - 1)
					{
						printf ("Ignoring 'r' argument: No folder specified\n");
						break;
					}
					if (argv[i+1][0] == '-')
					{
						printf ("Ignoring 'r' argument: No folder specified\n");
						break;
					}

					file_s *file = new file_s;
					file->folder = true;
					file->recursive = true;
					i++; // increase i.. we used that arg.
					file->name = argv[i];

					config.files.AddTail(file); // add to linked list
					break;
				}
// -f
				case 'f':
				{
#ifdef NO_MULTIARG_FILES
					if (arglen != 2)
					{
						printf ("Ignoring 'f' argument: Cannot be used in multiple argument list\n");
						break;
					}
#endif
					if (i == argc - 1)
					{
						printf ("Ignoring 'f' argument: No map specified\n");
						break;
					}
					if (argv[i+1][0] == '-')
					{
						printf ("Ignoring 'f' argument: No map specified\n");
						break;
					}

					file_s *file = new file_s;
					file->folder = false;
					file->recursive = false;
					i++; // increase i.. we used that arg.
					file->name = argv[i];

					config.files.AddTail(file); // add to linked list
					break;
				}
// -x
				case 'x':
				{
#ifdef NO_MULTIARG_FILES
					if (arglen != 2)
					{
						printf ("Ignoring 'x' argument: Cannot be used in multiple argument list\n");
						break;
					}
#endif
					if (i == argc - 1)
					{
						printf ("Ignoring 'x' argument: No map specified\n");
						break;
					}
					if (argv[i+1][0] == '-')
					{
						printf ("Ignoring 'x' argument: No map specified\n");
						break;
					}

					file_s *file = new file_s;
					file->folder = false;
					file->recursive = false;
					i++; // increase i.. we used that arg.
					file->name = argv[i];

					config.excludes.AddTail(file); // add to linked list
					break;
				}
// -o
				case 'o':
					config.overwrite = true;
					break;
// -a
				case 'a':
#ifdef NO_MULTIARG_FILES
					if (arglen != 2)
					{
						printf ("Ignoring 'a' argument: Cannot be used in multiple argument list\n");
						break;
					}
#endif
					if (i == argc - 1)
					{
						printf ("Ignoring 'a' argument: No rfa file specified\n");
						break;
					}
					if (argv[i+1][0] == '-')
					{
						printf ("Ignoring 'a' argument: No rfa file specified\n");
						break;
					}

					i++; // increase i.. we used that arg.
					config.rfafile = argv[i];
					break;
// -l
				case 'l':
					config.tolower = true;
					break;
// -m
				case 'm':
					config.matchcase = true;
					break;
// -e
				case 'e':
#ifdef NO_MULTIARG_FILES
					if (arglen != 2)
					{
						printf ("Ignoring 'e' argument: Cannot be used in multiple argument list\n");
						break;
					}
#endif
					if (i == argc - 1)
					{
						printf ("Ignoring 'e' argument: No folder specified\n");
						break;
					}
					if (argv[i+1][0] == '-')
					{
						printf ("Ignoring 'e' argument: No folder specified\n");
						break;
					}

					i++; // increase i.. we used that arg.
					config.resource_path = argv[i];
					break;
// -p
				case 'p':
					config.checkpak = false;
					break;
// -u
				case 'u':
					config.parseresource = true;
					break;
// -n
				case 'n':
					config.preservewads = true;
					break;
// -b
				case 'b':
#ifdef NO_MULTIARG_FILES
					if (arglen != 2)
					{
						printf ("Ignoring 'b' argument: Cannot be used in multiple argument list\n");
						break;
					}
#endif
					if (i == argc - 1)
					{
						printf ("Ignoring 'b' argument: No rfa file specified\n");
						break;
					}
					if (argv[i+1][0] == '-')
					{
						printf ("Ignoring 'b' argument: No rfa file specified\n");
						break;
					}

					i++; // increase i.. we used that arg.
					config.excludelists.AddTail(new VString(argv[i]));
					break;
#ifdef WIN32
// -k
				case 'k':
					config.keypress = false;
					break;
#else
// -t
				case 't':
					config.symlink = false;
					break;
#endif // WIN32

				default:
					printf("Ignoring '%c' argument: Argument not known\n", argstr[j]);
				}
			}
		}
		else
		{
			// not a switch. Assume it's a map.
			file_s *file = new file_s;
			file->folder = false;
			file->recursive = false;
			file->name = argv[i];

			config.files.AddTail(file); // add to linked list
		}
	}

	// Make sure statusline and folderlist doesn't display in non verbal mode
	if (!config.verbal)
	{
		config.statusline = false;
		config.searchdisp = false;
		config.resourcedisp = false;
		config.contentdisp = false;
	}

	// check for stuff to display
	if (config.warranty)
	{
		showwarranty();
#ifdef WIN32
		getexitkey(config.verbal, config.keypress);
#endif
		exit(0);
	}

	if (config.verbal)
	{
		showcopyright();
	}

	if (config.help)
	{
		showhelp();
#ifdef WIN32
		getexitkey(config.verbal, config.keypress);
#endif
		exit(0);
	}

	if (config.credits)
	{
		showcredits();
#ifdef WIN32
		getexitkey(config.verbal, config.keypress);
#endif
		exit(0);
	}

	// check if we have anything to do
	if (config.files.GetCount() == 0)
	{
		showhelp();

		printf("\nERROR: You need to specify at least 1 folder or map to process\n");
#ifdef WIN32
		getexitkey(config.verbal, config.keypress);
#endif
		exit(0);
	}

	if (!config.verbal)
	{
		printf("Generating RES files. Please stand by...\n");
	}

	// Build filelist
	LinkedList<VString *> FileList; // bsp files
	LinkedList<VString *> ErrorList; // failed bsp files
	LinkedList<VString *> MissingList; // bsp files with missing reources
	VString *strtmp;

	ListBuilder listbuild(&FileList, &config.excludes, config.verbal, config.searchdisp);
#ifndef WIN32
	listbuild.SetSymLink(config.symlink);
#endif
	listbuild.BuildList(&config.files);

	// clean up config.files, we don't need it anymore
	while (config.files.GetCount() > 0)
	{
		file_s *file = config.files.GetAt(0);
		config.files.RemoveAt(0);
		delete file;
	}

	// Clean up config.exludes, we don't need it anymore
	while (config.excludes.GetCount() > 0)
	{
		file_s *file = config.excludes.GetAt(0);
		config.excludes.RemoveAt(0);
		delete file;
	}

	if (config.verbal) { printf("\n"); } // Make output look a bit cleaner

	// list made. Now parse the res files.
	RESGen resgen;

	resgen.SetParams(
		config.verbal, config.statusline, config.overwrite, config.tolower,
		config.matchcase, config.parseresource, config.preservewads, config.contentdisp);

	if(!resgen.LoadRfaFile(config.rfafile))
	{
		// Could not load RFA file, exit
		#ifdef WIN32
		getexitkey(config.verbal, config.keypress);
		#endif
		return 0;
	}

	// Load all resource exclude lists
	if (config.excludelists.GetCount())
	{
		while(config.excludelists.GetCount())
		{
			strtmp = config.excludelists.GetAt(0);
			if (!resgen.LoadExludeFile(*strtmp))
			{
				#ifdef WIN32
				getexitkey(config.verbal,config.keypress);
				#endif
				return 0;
			}
			else
			{
				if (config.verbal)
				{
					printf("Loaded resource exclude list %s\n",(LPCSTR)*strtmp);
				}
			}
			config.excludelists.RemoveAt(0); // clean up, we don't need it after this
			delete strtmp;
		}

		if (config.verbal) { printf("\n"); }
	}

	resgen.BuildResourceList(config.resource_path, config.checkpak, config.searchdisp, config.resourcedisp);

	int i = 1;
	int filecount = FileList.GetCount();
	while (FileList.GetCount() > 0)
	{
		if (config.contentdisp) { printf("\n"); } // Make output look a bit cleaner

		int retval = resgen.MakeRES(*FileList.GetAt(0), i, filecount);
		if(retval)
		{
			if (retval == 2)
			{
				// res file was made properly, but some resources were missing
				strtmp = new VString(*FileList.GetAt(0));
				MissingList.AddTail(strtmp);
			}
			else
			{
				//
				// an error occured. List them.
				strtmp = new VString(*FileList.GetAt(0));
				ErrorList.AddTail(strtmp);

				resgen.ClearResfile(); // We are not going to try and fix it, so resfile can be cleared.
				resgen.ClearTextures(); // Also clear texture list.
			}
		}

		strtmp = FileList.GetAt(0);
		FileList.RemoveAt(0);
		delete strtmp;

		if (config.verbal) { printf("\n"); } // Make output look a bit cleaner

		i++; // keep i up to date!
	}

	// clean up errors
	int errorcount = ErrorList.GetCount();
	int missingcount = MissingList.GetCount();
	if (errorcount)
	{
		if (config.verbal) { printf("Failed to create res file(s) for:\n"); }
		while (ErrorList.GetCount() > 0)
		{
			strtmp = ErrorList.GetAt(0);
			if (config.verbal) { printf(" %s\n", (LPCSTR)*strtmp); } // only print of verbal
			ErrorList.RemoveAt(0);
			delete strtmp;
		}
		if (config.verbal) { printf("\n"); }
	}
	if (missingcount)
	{
		// res file might not be complete
		if (config.verbal) {
			printf("Because one or more required files were not found in your installation,\n");
			printf("the following map(s) might be missing resources:\n");
		}
		while (MissingList.GetCount() > 0)
		{
			strtmp = MissingList.GetAt(0);
			if (config.verbal) { printf(" %s\n", (LPCSTR)*strtmp); } // only print of verbal
			MissingList.RemoveAt(0);
			delete strtmp;
		}
		if (config.verbal) { printf("\n"); }
	}

	printf("Done creating res file(s)! %d map(s) were processed", filecount - errorcount);
	if (errorcount)
	{
		printf(", skipped %d due to errors.\n", errorcount);
	}
	else
	{
		printf(".\n");
	}

	if (missingcount)
	{
		printf("%d map(s) might be missing resources.\n", missingcount);
	}
	// res files made.. exit

	// note we don't bother to clean up memory, the OS will do this for us.

#ifdef WIN32
	getexitkey(config.verbal, config.keypress);
#endif
	return 0;
}
