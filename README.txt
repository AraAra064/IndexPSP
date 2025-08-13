=====IndexPSPArchiveEditor 1.1======
     Made with ConsoleGraphics 1.0

AA means Asset Archive (GRP, EVT etc)
!Might not correctly modify ADV AA, it hasn't been tested.

Available encoding versions are 2.5
Higher versions are more effecient at encoding data.

-INFO-

-Program detects input even though it isn't selected (scuffed code)
-When replacing an image, use a BMP/PNG file (can't be bothered to add other image types)
-When replacing other file types, such as SSAD or PSC6, use a binary file

-WARNINGS-

-Currently, the program only supports PNG and 24-bit BMP files.
-Model UV Scaler has not been implemented yet (might end up adding it to the IndexPSPCRAFEditor).

--HOW TO USE--

You must source your own copy of the asset archives! They must be copied into the "OriginalArchives" folder for the program to find them.
!You can also use this program though cmd, type "ipae.exe ?" to get more into about this.

-INPUTS-

When selecting an asset archive, use the UP and DOWN arrow keys to navigate and press ENTER to select it.
When modifying an AA:
	A = Add new file to asset archive (it will append it to the end of the AA)
	R = Replace current file
	X = Remove last file from AA
	J = Jump to a specific file in AA
	K = Create a list of files to replace
	L = Load a list of files to replace (just batch replaces files)
	C = Create new AA (saves changes made to AA puts it in the "CustomArchives" folder with the same name)
	E = Extract options (extracts all files in an AA with a few options, saved to the ".SavedFiles" folder)

-REPLACING A FILE-
1. Put new files in the ".NewFiles" folder.
2. Open the program and go to which file you'd like to replace.
3. Press the 'R' key and type in the name of the file you'd like to replace the file with.
   [Absolute paths will ***NOT*** work, like "C:\Users\*USER*\...\image.png"]

-ADDING A FILE-
   [Similar to replacing a file but you press the 'A' key]

-SAVING CHANGES AND ADDING TO ISO-
1. Press the 'C' key to create a new Asset Archive.
2. Use UMDGen, open the IndexPSP ISO, switch the AAs and save.
