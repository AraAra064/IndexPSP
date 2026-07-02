#include <string>
#include <vector>
#include <limits>

#include "ConsoleGraphics.hpp"
#include "SSAD.hpp"

#ifndef SSAD_RENDERER
#define SSAD_RENDERER

namespace IndexSSADRenderer
{
	const int32_t ANI_DEFAULT = std::numeric_limits<int32_t>::min();

	using IndexSSAD::UIElement;

	int32_t AniHandler(std::vector<uint32>& eData, int32 t)
	{
		int32_t val = 0;

		if (!eData.empty())
		{
			switch (eData[2])
			{
				case 0: //default value;
					val = ANI_DEFAULT;
					break;

				case 1: //static value
					val = eData[4];
					break;

				default: //animated
				{
					int32_t s = eData[2];
					int32_t index = 3, i = 0;
					int32_t s1, s2, st2;
					while (i < s - 1)
					{
						s1 = eData[index + 1];
						t = t > eData[index - 1] ? t - eData[index - 1] : t;
						index += 7;
						s2 = eData[index + 1];
						i++;
					}

					st2 = eData[index];
					//t %= st2;
					t = std::min(t, st2);
					val = map<float>(t, 0, st2, s1, s2); //must be float
				}
				break;
			}
		}

		return val;
	}

	int32 GetPos(int32 p)
	{
		return (p == ANI_DEFAULT ? 0 : 0.001f * p);
	}

	void PosInterpreter(cg::Image& img, UIElement& e, uint32 t)
	{
		int32_t posX = GetPos(AniHandler(e.posX.data, t));
		int32_t posY = GetPos(AniHandler(e.posY.data, t));

		img.setPos(posX, posY);

		return;
	}

	void UIScaleInterpreter(float& sx, float& sy, UIElement& e, uint32 t)
	{
		int32_t v = AniHandler(e.scaX.data, t);
		sx = (v == ANI_DEFAULT ? 1.f : (float)v * 0.001f); //div value by 1000
		v = AniHandler(e.scaY.data, t);
		sy = (v == ANI_DEFAULT ? 1.f : (float)v * 0.001f);

		return;
	}

	uint8 AlphaInterpreter(cg::Image& img, UIElement& e, uint32 t)
	{
		int32_t a = AniHandler(e.tran.data, t);
		if (a == ANI_DEFAULT){a = 1000;
		}
		float _a = (float)a / 1000.f;

		void (*AlphaFunc)(std::pair<uint32, uint8>*, void*) = [](std::pair<uint32, uint8>* p, void* d) -> void
		{
			float _a = *reinterpret_cast<float*>(d); //between 0 and 1
			p->second *= _a;
		};
		
		if (_a != 1.f)
		{
			img.filter(cg::FilterType::Custom, AlphaFunc, &_a);
		}
		else if (_a == 0.f)
		{
			img.setAlpha(0);
		}

		return a;
	}

	//Just going to use glColor when rewriting this
	void COLInterpreter(cg::Image& img, UIElement& e, uint32 t)
	{
		uint8_t cR = 255, cG = 255, cB = 255;

		void (*ColourFunc)(std::pair<uint32, uint8>*, void*) = [](std::pair<uint32, uint8>* p, void* d) -> void
		{
			return;
		};

		return;
	}
}

#endif
