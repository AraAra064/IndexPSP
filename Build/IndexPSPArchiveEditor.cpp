#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <direct.h>
#include <mutex>

#include "ConsoleGraphics.hpp"
#undef LoadImage
#undef ReplaceFile
#include "ConsoleUtilities.h"
#include "StringParser.hpp"

#include "IndexPSP/AssetArchive.hpp"
#include "IndexPSP/ShadeDecoder.hpp"
#include "IndexPSP/Encoder.hpp"
#include "IndexPSP/SHTXPS.hpp"
#include "IndexPSP/KadaTools.hpp"
#include "IndexPSP/CRAF.hpp"
#include "IndexPSPArchiveEditor.hpp"
#include "PipeHandler.hpp"

#include "Commands.cpp"
#include "CLI.cpp"
#include "SelectState.cpp"
#include "ModifyState.cpp"

int main(uint32 nArgs, char** arg)
{
	//IndexPSPArchiveEditor.exe CREATEAA AA_IN LISTNAME (AA_OUT??)
	//IndexPSPArchiveEditor.exe EXTRACT AA_IN FILEINDEX (FILE_OUT??)
	//IndexPSPArchiveEditor.exe EXTRACTALL AA_IN LISTNAME (AA_OUT??)
	//IndexPSPArchiveEditor.exe GRP_ENG AltTextureModB.txt (AA_OUT??/GRP.bin)
	
	std::string verStr = "1.1";
	cg::ConsoleGraphics *graphics = new cg::ConsoleGraphics(1920, 1080, true, 3, true);
	graphics->enableAlpha();
	graphics->setTitle("IndexPSPArchiveEditor v"+verStr);
	GlobalData global;
	//LoadSettings("Settings.txt", global.settings);

	if (nArgs > 1)
	{
		return cli(nArgs, arg);
	}

	StateManager sm(graphics, &global);
	sm.addState(new SelectState());
	sm.addState(new ModifyState());

	float dt;
	HighResClock c;

	while (sm.getState() != 0xFF)
	{
		dt = c.getElapsedTimeAsSeconds();
		c.restart();
		sm.update(dt);
		sm.render();
	}

	return 0;
}
