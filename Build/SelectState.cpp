#include "ConsoleGraphics.hpp"
#include "StateManager.hpp"

class SelectState : public StateBase
{
	const uint32 backgroundColour = 0x005F5FAF;
	const uint32 rowColour = 0x00AFAFAF;
	const uint8 rowAlpha = 0x3F;
	cg::Image mugino;
	bool loaded;
	cg::Image font;
	cg::Text text;
	uint32 archiveIndex;

	std::vector<std::string> archiveNames = {"GRP", "DAT", "EVT", "ADV", "FARC", "PSPSND", "PSPBGM"};
	std::vector<bool> encodedArchives = {true, true, true, false, false, false};

	Keyboard* keyboard;
	GlobalData* gd;

	public:
		SelectState()
		{
			keyboard = nullptr;
			gd = nullptr;

			archiveIndex = 0;
			loaded = false;
			LoadImage(mugino, "Images\\BestGirlMugino.png");
			mugino.scale(0.5f, cg::InterpolationMethod::AreaAveraging);
			font.loadImage("Fonts\\DefaultFontx2.bmp");
			font.setColourToAlpha(cg::BGR(255, 0, 0));
			text.setFont(&font, 8 * 2, 13 * 2);
		}
		//~SelectState()

		void onStartup(void)
		{
			gd = (GlobalData*)globalStateData;
			keyboard = &gd->keyboard;

			keyboard->setMaxDownTime(100);
			mugino.setPos(graphics->getWidth() - mugino.getWidth(), graphics->getHeight() - mugino.getHeight());
			return;
		}
		void update(float dT)
		{

			if (keyboard->isKeyDown(VK_UP) && archiveIndex != 0)
			{
				archiveIndex--;
			}
			else if (keyboard->isKeyDown(VK_DOWN) && archiveIndex != archiveNames.size() - 1)
			{
				archiveIndex++;
			}
			
			if (keyboard->isKeyDown(VK_RETURN))
			{
				std::string fileName = "OriginalArchives\\" + archiveNames[archiveIndex] + ".BIN"; //test
				gd->archiveFileName = fileName;
				gd->archiveName = archiveNames[archiveIndex];
				gd->archiveEncoded = encodedArchives[archiveIndex];
				gd->archiveIndex = archiveIndex;
				*currentState = 1;
				//gd->archiveData = LoadFile(fileName);
				//*stateManagerState = archiveIndex;
				//if (!(loaded = !gd->archiveData.empty()))
				//{
				//	//pipe.send("[ERROR:LOAD_ARCHIVE0] Can\'t load \"" + fileName + "\".\nCheck the \"OriginalArchives\" directory.");
				//	pause();
				//	*stateManagerState = -1;
				//}
			}

			if (keyboard->isKeyDown(VK_ESCAPE))
			{
				*stateManagerState = 0xFF;
			}

			if (loaded)
			{
				*currentState = 1;
			}

			keyboard->update(dT);
			return;
		}
		void render(void)
		{
			graphics->drawRect(0, 0, graphics->getWidth(), graphics->getHeight(), backgroundColour);
			graphics->draw(mugino);

			text.setPos(0, 0);
			text.setText("Select an Asset Archive to edit:");
			graphics->draw(text);

			uint32 y = text.getCharHeight() * 1.5f;
			uint32 index = 0;
			for (std::string& s : archiveNames)
			{
				text.setText(s);
				text.setPos(0, y);
				if (index == archiveIndex) {
					graphics->drawRectA(text.getPosX(), text.getPosY(), text.getWidth(), text.getHeight(), ~backgroundColour, rowAlpha);
				}
				
				graphics->draw(text);
				y += text.getCharHeight();
				index++;
			}
		}
};