//ConsoleGraphics V1.1
//ISO C++ 11 or higher must be used to compile ConsoleGraphics.

#include <vector>
#include <limits>
#include <cmath>
#include <string>
#include <fstream>
#include <algorithm>
#include <utility>

#define NOMINMAX
#include <windows.h>

#ifdef CG_DEBUG
	#include <iostream>
#endif

#ifndef int8
	#define int8 int8_t
	#define uint8 uint8_t
	#define int16 int16_t
	#define uint16 uint16_t
	#define int32 int32_t
	#define uint32 uint32_t
	#define int64 int64_t
	#define uint64 uint64_t
#endif

#ifndef CG_INCLUDE
#define CG_INCLUDE

//cg::BGR() is the same as RGB() from windows.h
#undef RGB

namespace cg
{
	class ConsoleGraphics;

	uint32 RGB(uint8 r, uint8 g, uint8 b){return r | (g << 8) | (b << 16);
	}
	uint32 BGR(uint8 r, uint8 g, uint8 b){return RGB(b, g, r);
	}
	uint32 RGBA(uint8 r, uint8 g, uint8 b, uint8 a){return RGB(r, g, b) | (a << 24);
	}
	uint32 RGBA(uint32 rgb, uint8 a){return (rgb & 0x00FFFFFF) | (a << 24);
	}
	uint32 BGRA(uint8 r, uint8 g, uint8 b, uint8 a){return RGBA(b, g, r, a);
	}
	uint32 BGRA(uint32 bgr, uint8 a){return (bgr & 0x00FFFFFF) | (a << 24);
	}
	
	//If isRGB == false, then input is assumed to be in the BGRA format
	uint8 GetR(uint32 rgba, bool isRGB = false){return isRGB ? rgba : rgba >> 16;
	}
	uint8 GetG(uint32 rgba){return rgba >> 8;
	}
	uint8 GetB(uint32 rgba, bool isRGB = false){return isRGB ? rgba >> 16 : rgba;
	}
	uint8 GetA(uint32 rgba){return rgba >> 24;
	}

	uint32 blendPixel(uint32 dstRGB, uint32 srcRGB, uint8 srcA)
	{
		static uint16 r, g, b;

		r = (cg::GetR(dstRGB) * (255 - srcA)) + (cg::GetR(srcRGB) * srcA);
		g = (cg::GetG(dstRGB) * (255 - srcA)) + (cg::GetG(srcRGB) * srcA);
		b = (cg::GetB(dstRGB) * (255 - srcA)) + (cg::GetB(srcRGB) * srcA);
		r /= 255, g /= 255, b /= 255;

		return cg::BGR((uint8)r, (uint8)g, (uint8)b);
	}

	enum class InterpolationMethod {None, NearestNeighbor, Bilinear, Bicubic, Bisinusoidal, AreaAveraging};
	enum class ExtrapolationMethod {None, Repeat, Extend};

	enum class FilterType {Grayscale, WeightedGrayscale, Invert, Custom = 255};

	enum class RenderMode {BitBlt, BitBltInv, SetPixel, SetPixelVer, SetPixelInv, SetPixelVerInv};
	enum class DrawType {Repeat, Resize};

	class Image
	{
		//First element in pair is for rgb data, the second is for alpha
		std::vector<std::pair<uint32, uint8>> pixels;
		uint32 width, height, x = 0, y = 0;
		float aspectRatio;

	protected:
		std::vector<std::pair<uint32, uint8>> ResizeData(std::vector<std::pair<uint32, uint8>>& pixels, uint32 newWidth, uint32 newHeight, InterpolationMethod m)
		{
			std::vector<std::pair<uint32, uint8>> data;
			bool success = true;
			float xScale = (float)width / (float)newWidth, yScale = (float)height / (float)newHeight;
			data.resize(newWidth * newHeight);

			switch (m)
			{
				default:
				case InterpolationMethod::NearestNeighbor:
				{
					uint32 srcX, srcY;
					for (uint32 y = 0; y < newHeight; ++y)
					{
						for (uint32 x = 0; x < newWidth; ++x)
						{
							srcX = x * xScale;
							srcY = y * yScale;
							data[(y * newWidth) + x] = pixels[(srcY * width) + srcX];
						}
					}
				}
				break;
				
				case InterpolationMethod::Bilinear:
				case InterpolationMethod::Bicubic:
				case InterpolationMethod::Bisinusoidal:
				{
					float srcX, srcY;
					for (uint32 y = 0; y < newHeight; ++y)
					{
						for (uint32 x = 0; x < newWidth; ++x)
						{
							srcX = x * xScale;
							srcX /= width;
							srcY = y * yScale;
							srcY /= height;
							data[(y * newWidth) + x] = samplePixel(srcX, srcY, m, ExtrapolationMethod::Extend);
						}
					}
				}
				break;
				
				case InterpolationMethod::AreaAveraging:
				{
					if ((success = (xScale > 1.f && yScale > 1.f)))
					{
						uint16 pixW = xScale, pixH = yScale;
						uint16 _w = pixW / 2, _h = pixH / 2;
						pixW = std::max<uint16>(pixW, 2);
						pixH = std::max<uint16>(pixH, 2);
						uint32 avR, avG, avB, avA;
						float alphaRatio;
						uint32 srcX, srcY;
						uint16 div = pixW * pixH;
						
						for (uint32 y = 0; y < newHeight; ++y)
						{
							for (uint32 x = 0; x < newWidth; ++x)
							{
								srcX = x * xScale;
								srcY = y * yScale;
								avR = 0, avG = 0, avB = 0, avA = 0;
								
								for (uint32 w = 0; w < pixW; ++w)
								{
									for (uint32 h = 0; h < pixH; ++h)
									{
										if (getPixel(srcX + w - _w, srcY + h - _h) != nullptr)
										{
											alphaRatio = pixels[((srcY + h - _h) * width) + (srcX + w - _w)].second / 255.f;
											avR += cg::GetR(pixels[((srcY + h - _h) * width) + (srcX + w - _w)].first) * alphaRatio;
											avG += cg::GetG(pixels[((srcY + h - _h) * width) + (srcX + w - _w)].first) * alphaRatio;
											avB += cg::GetB(pixels[((srcY + h - _h) * width) + (srcX + w - _w)].first) * alphaRatio;
											avA += pixels[((srcY + h - _h) * width) + (srcX + w - _w)].second;
										} else {
											alphaRatio = pixels[(srcY * width) + srcX].second / 255.f;
											avR += cg::GetR(pixels[(srcY * width) + srcX].first) * alphaRatio;
											avG += cg::GetG(pixels[(srcY * width) + srcX].first) * alphaRatio;
											avB += cg::GetB(pixels[(srcY * width) + srcX].first) * alphaRatio;
											avA += pixels[(srcY * width) + srcX].second;
										}
									}
								}
								
								avR /= div, avG /= div, avB /= div, avA /= div;
								data[(y * newWidth) + x] = std::make_pair(RGB(avB, avG, avR), avA);
							}
						}
					} else data = this->pixels;
				}
				break;
			}
			if (success)
			{
				width = newWidth;
				height = newHeight;
			} else {
				#ifdef CG_DEBUG
					std::cerr << "CGIMG ERROR {this->ResizeData()}: New image size greater than original [" << width << char(158) << height << " -> " << newWidth << char(158) << newHeight << ", InterpolationMethod::AreaAveraging]" << std::endl;
				#endif
			}
			return data;
		}

		void GetWindowSize(HWND w, uint32& width, uint32& height)
		{
			RECT window;
			GetWindowRect(w, &window);

			width = window.right - window.left;
			height = window.bottom - window.top;
			return;
		}

	public:
		//Default constructor
		Image()
		{
			width = 256;
			height = 256;
			aspectRatio = 1.f;
			pixels.resize(width * height);
			memset(pixels.data(), 0, pixels.size() * sizeof(std::pair<uint32, uint8>));
		}
		//Create an image and fill image with a colour
		Image(uint32 width, uint32 height, uint32 rgb = 0, uint8 a = 255)
		{
			this->width = width;
			this->height = height;
			aspectRatio = (float)width / (float)height;
			pixels.resize(width * height);
			for (uint32 y = 0; y < height; ++y)
			{
				for (uint32 x = 0; x < width; ++x)
				{
					pixels[(y * width) + x] = std::make_pair(rgb, a);
				}
			}
		}
		//Load image from memory
		Image(const uint32* data, uint32 width, uint32 height, bool alpha = false)
		{
			loadImageFromArray(data, width, height, alpha);
		}
		//Load image from memory
		Image(const std::pair<uint32, uint8>* data, uint32 width, uint32 height)
		{
			loadImageFromArray(data, width, height);
		}
		//Load image from file
		Image(const std::string fileName)
		{
			loadImage(fileName);
		}

		//Copy constructor
		Image(const Image& image)
		{
			width = image.getWidth();
			height = image.getHeight();
			aspectRatio = (float)width / (float)height;
			x = image.getPosX();
			y = image.getPosY();

			pixels.resize(width * height);
			memcpy(&pixels[0], image.getPixelData(), width * height * sizeof(std::pair<uint32, uint8>));
		}
		Image& operator=(const Image& image)
		{
			if (&image != this)
			{
				width = image.getWidth();
				height = image.getHeight();
				aspectRatio = (float)width / (float)height;
				x = image.getPosX();
				y = image.getPosY();

				pixels.resize(width * height);
				memcpy(&pixels[0], image.getPixelData(), width * height * sizeof(std::pair<uint32, uint8>));
			}
			return *this;
		}

		std::pair<uint32, uint8>* operator[](uint32 i)
		{
			if (i < width * height){return &pixels[i];
			} else return nullptr;
		}

		//Loads an image from the disk (currently only supports 24 and 32 bit BMP files)
		bool loadImage(const std::string fileName)
		{
			std::string _fileName = fileName;
			std::transform(_fileName.begin(), _fileName.end(), _fileName.begin(), [](char c)->char {return toupper(c);});

			if (_fileName.find(".BMP") != std::string::npos)
			{
				std::ifstream readfile(fileName, std::ios::binary);
				if (!readfile.is_open())
				{
					#ifdef CG_DEBUG
						std::cerr << "CGIMG ERROR {this->loadImage()}: Failed to read file [" << fileName << ", errno=" << errno << "]" << std::endl;
					#endif
					return false;
				}

				//Read header
				std::vector<uint8> header(14);
				readfile.read((char*)&header[0], 14 * sizeof(uint8));
				uint32 fileSize = *(uint32*)&header[0x02];
				uint32 imgDataOffset = *(uint32*)&header[0x0A];
				uint32 dibHeaderSize = imgDataOffset - header.size();
				header.resize(imgDataOffset);
				readfile.read((char*)&header[14], dibHeaderSize * sizeof(uint8));

				int32 width = *(int32*)&header[0x12];
				int32 height = *(int32*)&header[0x16];

				uint16 bitesPerPixel = *(uint16*)&header[0x1C];
				uint16 bytesPerPixel = bitesPerPixel / 8;
				if (bytesPerPixel < 1){bytesPerPixel = 1;
				}

				//Check if valid before loading
				if (header[0] != 'B' || header[1] != 'M')
				{
					#ifdef CG_DEBUG
						std::cerr << "CGIMG ERROR {this->loadImage()}: Invalid BMP magic number [" << fileName << "]" << std::endl;
					#endif
					return false;
				}

				if (bytesPerPixel < 3)
				{
					#ifdef CG_DEBUG
						std::cerr << "CGIMG ERROR {this->loadImage()}: Currently only supports 24 or 32 bit BMP files [" << fileName << "]" << std::endl;
					#endif
					return false;
				}// else std::cout << "BytesPerPixel=" << bytesPerPixel << std::endl;

				this->width = width;
				this->height = abs(height);
				aspectRatio = (float)this->width / (float)this->height;
				pixels.resize(this->width * this->height);

				const uint32 rowSize = width * bytesPerPixel, paddingSize = (4 - (width % 4)) % 4;
				std::vector<uint8> imgData(fileSize - imgDataOffset);
				readfile.read(reinterpret_cast<char*>(&imgData[0]), imgData.size() * sizeof(uint8));

				for (uint32 y = 0; y < this->height; ++y)
				{
					for (uint32 x = 0; x < width; ++x)
					{
						uint32 index = ((rowSize + paddingSize) * y) + (x * bytesPerPixel);
						uint8 red = imgData[index + 2];
						uint8 green = imgData[index + 1];
						uint8 blue = imgData[index];
						uint8 alpha = (bytesPerPixel == 3 ? 255 : imgData[index + 3]);
						pixels[(y * this->width) + x] = std::make_pair(cg::BGR(red, green, blue), alpha);
					}
				}

				readfile.close();
				if (height > 0){flipVertically();
				}
			} else return false;
			return true;
		}

		//Loads image from memory, format = 0xAARRGGBB
		void loadImageFromArray(const uint32* arr, uint32 width, uint32 height, bool alpha = false)
		{
			pixels.resize(width * height);
			this->width = width;
			this->height = height;
			this->aspectRatio = (float)width / (float)height;

			for (uint32 y = 0; y < height; ++y)
			{
				for (uint32 x = 0; x < width; ++x)
				{
					const uint32& pixel = arr[(y * width) + x];
					uint32 rgb = cg::BGR(cg::GetR(pixel), cg::GetG(pixel), cg::GetB(pixel));
					uint8 a = alpha ? cg::GetA(pixel) : 255;
					pixels[(y * width) + x] = std::make_pair(rgb, a);
				}
			}
			return;
		}
		//Loads image from memory, format = {0x00RRGGBB, 0xAA}
		void loadImageFromArray(const std::pair<uint32, uint8>* arr, uint32 width, uint32 height)
		{
			pixels.resize(width * height);
			this->width = width;
			this->height = height;
			this->aspectRatio = (float)width / (float)height;

			for (uint32 y = 0; y < height; ++y)
			{
				for (uint32 x = 0; x < width; ++x)
				{
					const uint32& rgb = arr[(y * width) + x].first;
					const uint8& a = arr[(y * width) + x].second;
					pixels[(y * width) + x] = std::make_pair(cg::BGR(cg::GetR(rgb), cg::GetG(rgb), cg::GetB(rgb)), a);
				}
			}
			return;
		}

		//Saves image to disk (ver parameter currently unused)
		void saveImage(const std::string fileName, uint32 ver = 0)
		{
			std::string _fileName = fileName;
			std::transform(_fileName.begin(), _fileName.end(), _fileName.begin(), [](char c)->char {return toupper(c);});

			if (_fileName.find(".BMP"))
			{
				std::vector<uint8> bmpHeader(14), dibHeader(40), pixData;
				memcpy(&bmpHeader[0x00], "BM", 2 * sizeof(char));
				uint32 val = bmpHeader.size() + dibHeader.size();
				memcpy(&bmpHeader[0x0A], &val, sizeof(uint32));

				val = 40;
				int32 width = this->width;
				int32 height = this->height;
				height = -height;
				memcpy(&dibHeader[0x00], &val, sizeof(uint32));
				memcpy(&dibHeader[0x04], &width, sizeof(int32));
				memcpy(&dibHeader[0x08], &height, sizeof(int32));
				val = 1;
				memcpy(&dibHeader[0x0C], &val, sizeof(uint16));
				val = 32;
				memcpy(&dibHeader[0x0E], &val, sizeof(uint16));

				const uint16 bytesPerPixel = 4;
				const uint32 rowSize = this->width * bytesPerPixel, paddingSize = (4 - (this->width % 4)) % 4;
				const uint8 paddingVal = 0x00;
				pixData.reserve(this->height * (rowSize + paddingSize));

				for (uint32 y = 0; y < this->height; y++)
				{
					for (uint32 x = 0; x < this->width; x++)
					{
						//BBGGRRAA
						auto p = accessPixel(x, y);
						pixData.push_back(GetB(p->first));
						pixData.push_back(GetG(p->first));
						pixData.push_back(GetR(p->first));
						pixData.push_back((p->second));
					}

					pixData.resize(pixData.size() + paddingSize, paddingVal);
				}

				//fileSize
				val = bmpHeader.size() + dibHeader.size() + pixData.size();
				memcpy(&bmpHeader[0x02], &val, sizeof(uint32));
				val = pixData.size();
				memcpy(&dibHeader[0x14], &val, sizeof(uint32));

				std::ofstream writeFile(fileName.c_str(), std::ios::binary);
				if (writeFile.is_open())
				{
					writeFile.write((char*)&bmpHeader[0], bmpHeader.size() * sizeof(char));
					writeFile.write((char*)&dibHeader[0], dibHeader.size() * sizeof(char));
					writeFile.write((char*)&pixData[0], pixData.size() * sizeof(char));
					writeFile.close();
				}
			}
			else {
				#ifdef CG_DEBUG
					std::cerr << "CGIMG ERROR {this->saveImage()}: Image type not supported. [" << fileName << "]" << std::endl;
				#endif
			}

			return;
		}

		//FilterType::Invert = Invert all colours
		//FilterType::Custom = Custom (pass in a function pointer, or lamda)
		//Applies a function to all pixels in the image, funcData is not required
		void filter(FilterType filterType, void (*funcPtr)(std::pair<uint32, uint8>*, void*) = nullptr, void* funcData = nullptr)
		{
			switch (filterType)
			{
				default:
				case FilterType::Grayscale:
				{
					uint8 r, g, b;
					uint16 c;
					for (uint32 i = 0; i < pixels.size(); ++i)
					{
						r = cg::GetR(pixels[i].first);
						g = cg::GetG(pixels[i].first);
						b = cg::GetB(pixels[i].first);
						c = (r + g + b) / 3;
						pixels[i].first = cg::BGR(c, c, c);
					}
				}
				break;
				
				case FilterType::WeightedGrayscale:
				{
					uint8 r, g, b, c;
					for (uint32 i = 0; i < pixels.size(); ++i)
					{
						r = cg::GetR(pixels[i].first);
						g = cg::GetG(pixels[i].first);
						b = cg::GetB(pixels[i].first);
						c = (r * 0.3f) + (g * 0.59f) + (b * 0.11f);
						pixels[i].first = cg::BGR(c, c, c);
					}
				}
				break;

				case FilterType::Invert:
				{
					for (uint32 i = 0; i < pixels.size(); ++i)
					{
						pixels[i].first = pixels[i].first ^ 0xFFFFFF;
					}
				}
				break;

				case FilterType::Custom:
				{
					for (uint32 i = 0; i < pixels.size(); ++i)
					{
						funcPtr(&pixels[i], funcData);
					}
				}
				break;
			}
			return;
		}

		//Returns the width of the image
		uint32 getWidth(void) const {return width;
		}
		//Returns the height of the image
		uint32 getHeight(void) const {return height;
		}

		//Sets the position of where the image will be drawn to (co-ordinates can't be negative)
		void setPos(uint32 x, uint32 y)
		{
			this->x = x;
			this->y = y;
			return;
		}
		//Moves the position of the image (co-ordinates can't be negative)
		void move(uint32 x, uint32 y)
		{
			this->x += x;
			this->y += y;
			return;
		}

		//Returns X co-ordinate of the image
		uint32 getPosX(void) const {return x;
		}
		//Returns Y co-ordinate of the image
		uint32 getPosY(void) const {return y;
		}

		//Flips an image on the X-axis
		void flipVertically(void)
		{
			uint32 halfHeight = height / 2;
			for (uint32 y = 0; y < halfHeight; ++y)
			{
				for (uint32 x = 0; x < width; ++x)
				{
					std::swap(pixels[(y * width) + x], pixels[((height - y - 1) * width) + x]);
				}
			}
			return;
		}
		//Flips an image on the Y-axis
		void flipHorizontally(void)
		{
			uint32 halfWidth = width / 2;
			for (uint32 y = 0; y < height; ++y)
			{
				for (uint32 x = 0; x < halfWidth; ++x)
				{
					std::swap(pixels[(y * width) + x], pixels[(y * width) + (width - x - 1)]);
				}
			}
			return;
		}

		//Return a read only pointer to pixel array
		const std::pair<uint32, uint8>* getPixelData(void) const {return this->pixels.data();
		}

		//If either newWidth or newHeight == 0, the image's aspect ratio is maintained
		//Resamples image to specified dimensions using chosen interpolation method
		void resize(uint32 newWidth, uint32 newHeight, InterpolationMethod m = InterpolationMethod::NearestNeighbor)
		{
			if (newWidth == 0)
			{
				pixels = ResizeData(pixels, newHeight * aspectRatio, newHeight, m);
				width = newHeight * aspectRatio;
				height = newHeight;
			}
			else if (newHeight == 0)
			{
				pixels = ResizeData(pixels, newWidth, newWidth / aspectRatio, m);
				width = newWidth;
				height = newWidth / aspectRatio;
			}
			else {
				pixels = ResizeData(pixels, newWidth, newHeight, m);
				width = newWidth;
				height = newHeight;
				aspectRatio = (float)width / (float)height;
			}
			return;
		}

		//Resamples image by a scale factor using a chosen interpolation method, aspect ratio is maintained
		void scale(float s, InterpolationMethod m = InterpolationMethod::NearestNeighbor)
		{
			if (s > 0.f)
			{
				resize(width * s, height * s, m);
			}
			return;
		}

		//Resamples image by a scale factor in each axis using a chosen interpolation method, aspect may be changed
		void scale(float sx, float sy, InterpolationMethod m = InterpolationMethod::NearestNeighbor)
		{
			if (sx > 0.f && sy > 0.f)
			{
				resize(width * sx, height * sy, m);
			}
			return;
		}

		//Returns a pointer to a pixel without checking if it exists 
		std::pair<uint32, uint8>* accessPixel(uint32 x, uint32 y){return &pixels[(y * width) + x];
		}

		//Returns a pointer to a pixel, if pixel doesn't exist "nullptr" will be returned
		std::pair<uint32, uint8>* getPixel(uint32 x, uint32 y)
		{
			if (x < width && y < height){return &pixels[(y * width) + x];
			} else return nullptr;
		}

		//Returns a copy of pixel at a point
		std::pair<uint32, uint8> samplePixel(float x, float y, InterpolationMethod im = InterpolationMethod::NearestNeighbor, ExtrapolationMethod em = ExtrapolationMethod::Repeat) const
		{
			std::pair<uint32, uint8> pixel;
			uint8 r[] = {0x00, 0x00, 0x00, 0x00};
			uint8 g[] = {0x00, 0x00, 0x00, 0x00};
			uint8 b[] = {0x00, 0x00, 0x00, 0x00};
			uint8 a[] = {0xFF, 0xFF, 0xFF, 0xFF};

			bool inBounds = (im != InterpolationMethod::AreaAveraging);
			bool badX = (x < 0.f || x > 1.f);
			bool badY = (y < 0.f || y > 1.f);

			if (badX || badY)
			{
				switch (em)
				{
					case cg::ExtrapolationMethod::None:
						inBounds = false;
						pixel = std::make_pair(0x00000000, 0x00);
						break;

					case cg::ExtrapolationMethod::Repeat:
						x = (x < 0.f ? x + 1.f : x - std::floor(x));
						y = (y < 0.f ? y + 1.f : y - std::floor(y));
						break;

					case cg::ExtrapolationMethod::Extend:
						x = std::min(std::max(0.f, x), 1.f);
						y = std::min(std::max(0.f, y), 1.f);
						break;
				}
			}

			float posX = x * (width - 1), posY = y * (height - 1);
			uint32 posXLower = posX, posXUpper = 0;
			uint32 posYLower = posY, posYUpper = 0;
			float valX = posX - std::floor(posX), valY = posY - std::floor(posY); //values used for interpolation

			if (im == InterpolationMethod::NearestNeighbor || im == InterpolationMethod::None)
			{
				inBounds = false;
				pixel = pixels[(std::round(posY) * width) + std::round(posX)];
			}

			if (inBounds)
			{
				switch (em)
				{
					case cg::ExtrapolationMethod::None:
						break;

					case cg::ExtrapolationMethod::Repeat:
						posXUpper = (posXLower + 1) % width;
						posYUpper = (posYLower + 1) % height;
						break;

					case cg::ExtrapolationMethod::Extend:
						posXUpper = std::min<uint32>(std::max<uint32>(0, posXLower + 1), width - 1);
						posYUpper = std::min<uint32>(std::max<uint32>(0, posYLower + 1), height - 1);
						break;
				}

				uint32 index = (posYLower * width) + posXLower;
				r[0] = cg::GetR(pixels[index].first);
				g[0] = cg::GetG(pixels[index].first);
				b[0] = cg::GetB(pixels[index].first);
				a[0] = pixels[index].second;

				index = (posYLower * width) + posXUpper;
				r[1] = cg::GetR(pixels[index].first);
				g[1] = cg::GetG(pixels[index].first);
				b[1] = cg::GetB(pixels[index].first);
				a[1] = pixels[index].second;

				index = (posYUpper * width) + posXLower;
				r[2] = cg::GetR(pixels[index].first);
				g[2] = cg::GetG(pixels[index].first);
				b[2] = cg::GetB(pixels[index].first);
				a[2] = pixels[index].second;

				index = (posYUpper * width) + posXUpper;
				r[3] = cg::GetR(pixels[index].first);
				g[3] = cg::GetG(pixels[index].first);
				b[3] = cg::GetB(pixels[index].first);
				a[3] = pixels[index].second;

				int16 dR, dG, dB, dA;
				float val, scale = -6.f; //for bicubic

				switch (im)
				{
					case cg::InterpolationMethod::Bilinear:
						dR = r[1] - r[0];
						dG = g[1] - g[0];
						dB = b[1] - b[0];
						dA = a[1] - a[0];
						r[0] = r[0] + (valX * dR); //Combined rX1
						g[0] = g[0] + (valX * dG); //Combined gX1
						b[0] = b[0] + (valX * dB); //Combined bX1
						a[0] = a[0] + (valX * dA); //Combined aX1

						dR = r[3] - r[2];
						dG = g[3] - g[2];
						dB = b[3] - b[2];
						dA = a[3] - a[2];
						r[1] = r[2] + (valX * dR); //Combined rX2
						g[1] = g[2] + (valX * dG); //Combined gX2
						b[1] = b[2] + (valX * dB); //Combined bX2
						a[1] = a[2] + (valX * dA); //Combined aX2

						dR = r[1] - r[0];
						dG = g[1] - g[0];
						dB = b[1] - b[0];
						dA = a[1] - a[0];
						r[0] = r[0] + (valY * dR); //Combined rY
						g[0] = g[0] + (valY * dG); //Combined gY
						b[0] = b[0] + (valY * dB); //Combined bY
						a[0] = a[0] + (valY * dA); //Combined aY

						pixel = std::make_pair(cg::BGR(r[0], g[0], b[0]), a[0]);
						break;

					case cg::InterpolationMethod::Bicubic: //Doesn't work correctly. "Bisinusiodal" should provide similar results.
						val = (0.333f * std::powf(valX, 3.f)) + (0.5f * std::powf(valX, 2.f));
						val *= scale;
						dR = r[1] - r[0];
						dG = g[1] - g[0];
						dB = b[1] - b[0];
						dA = a[1] - a[0];
						r[0] = (dR * val) + r[0]; //Combined rX1
						g[0] = (dG * val) + g[0]; //Combined gX1
						b[0] = (dB * val) + b[0]; //Combined bX1
						a[0] = (dA * val) + a[0]; //Combined aX1

						dR = r[3] - r[2];
						dG = g[3] - g[2];
						dB = b[3] - b[2];
						dA = a[3] - a[2];
						r[1] = (dR * val) + r[2]; //Combined rX2
						g[1] = (dG * val) + g[2]; //Combined gX2
						b[1] = (dB * val) + b[2]; //Combined bX2
						a[1] = (dA * val) + a[2]; //Combined aX2

						val = (0.333f * std::pow(valY, 3.f)) + (0.5f * std::pow(valY, 2.f));
						val *= scale;
						dR = r[1] - r[0];
						dG = g[1] - g[0];
						dB = b[1] - b[0];
						dA = a[1] - a[0];
						r[0] = (dR * val) + r[0]; //Combined rY
						g[0] = (dG * val) + g[0]; //Combined gY
						b[0] = (dB * val) + b[0]; //Combined bY
						a[0] = (dA * val) + a[0]; //Combined aY

						pixel = std::make_pair(cg::BGR(r[0], g[0], b[0]), a[0]);
						break;

					case cg::InterpolationMethod::Bisinusoidal:
						val = (-0.5f * std::cosf(3.14159f * valX)) + 0.5f;
						dR = r[1] - r[0];
						dG = g[1] - g[0];
						dB = b[1] - b[0];
						dA = a[1] - a[0];
						r[0] = (dR * val) + r[0]; //Combined rX1
						g[0] = (dG * val) + g[0]; //Combined gX1
						b[0] = (dB * val) + b[0]; //Combined bX1
						a[0] = (dA * val) + a[0]; //Combined aX1

						dR = r[3] - r[2];
						dG = g[3] - g[2];
						dB = b[3] - b[2];
						dA = a[3] - a[2];
						r[1] = (dR * val) + r[2]; //Combined rX2
						g[1] = (dG * val) + g[2]; //Combined gX2
						b[1] = (dB * val) + b[2]; //Combined bX2
						a[1] = (dA * val) + a[2]; //Combined aX2

						val = (-0.5f * std::cosf(3.14159f * valY)) + 0.5f;
						dR = r[1] - r[0];
						dG = g[1] - g[0];
						dB = b[1] - b[0];
						dA = a[1] - a[0];
						r[0] = (dR * val) + r[0]; //Combined rY
						g[0] = (dG * val) + g[0]; //Combined gY
						b[0] = (dB * val) + b[0]; //Combined bY
						a[0] = (dA * val) + a[0]; //Combined aY

						pixel = std::make_pair(cg::BGR(r[0], g[0], b[0]), a[0]);
						break;

					default:
						break;
				}
			}
			
			return pixel;
		}

		void setPixel(uint32 x, uint32 y, uint32 rgb, uint8 a = 255)
		{
			if (x < width && y < height)
			{
				pixels[(y * width) + x] = std::make_pair(rgb, a);
			}
			return;
		}

		//Sets all alpha values in the image
		void setAlpha(uint8 a)
		{
			for (uint32 i = 0; i < pixels.size(); ++i)
			{
				pixels[i].second = a;
			}
			return;
		}

		//Replaces the alpha value of all pixels with certain colour
		void setColourToAlpha(uint32 rgb, uint8 a = 0)
		{
			for (uint32 i = 0; i < pixels.size(); ++i)
			{
				if (pixels[i].first == rgb){pixels[i].second = a;
				}
			}
			return;
		}

		//Change the image's dimensions (does not resample image)
		void setSize(uint32 newWidth, uint32 newHeight, bool clearData = false)
		{
			auto temp = pixels;
			pixels.clear();
			pixels.resize(newWidth * newHeight);
			if (!clearData)
			{
				for (uint32 y = 0; y < height; y++)
				{
					for (uint32 x = 0; x < width; x++)
					{
						pixels[(y * newWidth) + x] = temp[(y * width) + x];
					}
				}
			}
			width = newWidth;
			height = newHeight;
			aspectRatio = (float)width / (float)height;
			return;
		}

		//Copy a section from a section of an image, to another (currently very slow)
		void copy(Image& image, uint32 dstX, uint32 dstY, uint32 srcX, uint32 srcY, uint32 width, uint32 height, bool alpha = false)
		{
			for (uint32 iy = 0; iy < height; iy++)
			{
				for (uint32 ix = 0; ix < width; ix++)
				{
					if (getPixel(ix + dstX, iy + dstY) != nullptr && image.getPixel(ix + srcX, iy + srcY) != nullptr)
					{
						pixels[((iy + dstY) * this->width) + (ix + dstX)] = std::make_pair(image.getPixelData()[((iy + srcY) * image.getWidth()) + (ix + srcX)].first, alpha ? image.getPixelData()[((iy + srcY) * image.getWidth()) + (ix + srcX)].second : 255);
					}
				}
			}
			return;
		}

		//Draws a section from an section, to another
		void blendImage(Image& image, uint32 dstX, uint32 dstY, uint32 srcX, uint32 srcY, uint32 width, uint32 height, bool keepAlpha = true, bool mask = true)
		{
			uint8 tempA;
			for (uint32 iy = 0; iy < height; iy++)
			{
				for (uint32 ix = 0; ix < width; ix++)
				{
					if (getPixel(ix + dstX, iy + dstY) != nullptr && image.getPixel(ix + srcX, iy + srcY) != nullptr)
					{
						tempA = pixels[((iy + dstY) * this->width) + (ix + dstX)].second;
						std::pair<uint32, uint8> srcRGB = image.getPixelData()[((iy + srcY) * image.getWidth()) + (ix + srcX)], & dstRGB = pixels[((iy + dstY) * this->width) + (ix + dstX)];
						dstRGB.first = blendPixel(dstRGB.first, srcRGB.first, srcRGB.second);
						dstRGB.second = keepAlpha ? dstRGB.second : srcRGB.second;
						if (mask && srcRGB.second == 0){dstRGB.second = tempA;
						}
					}
				}
			}
			return;
		}
	};

	struct Size
	{
		uint32 width, height;

		Size()
		{
			width = 0, height = 0;
		}
		Size(uint32 width, uint32 height)
		{
			this->width = width;
			this->height = height;
		}
	};

	struct SubImageData
	{
		Size size;
		uint32 srcX, srcY, dstX, dstY;
		bool isVertical, transparency;

		SubImageData()
		{
			srcX = 0, srcY = 0;
			dstX = 0, dstY = 0;
			isVertical = false;
			transparency = false;
		}
		SubImageData(uint32 srcX, uint32 srcY, uint32 dstX, uint32 dstY, uint32 width, uint32 height, bool isVertical, bool transparency = false)
		{
			this->srcX = srcX;
			this->srcY = srcY;
			this->dstX = dstX;
			this->dstY = dstY;
			size.width = width;
			size.height = height;
			this->isVertical = isVertical;
			this->transparency = transparency;
		}
	};

	class Text
	{
		Image* font = nullptr;
		std::string text;
		uint32 charWidth, charHeight;
		Image textImage;

		protected:
	
		void getTextSize(std::string text, uint32& width, uint32& height)
		{
			width = 0;
			height = 1;

			std::vector<uint32> sizes;

			for (uint32 i = 0; i < text.size(); ++i)
			{
				if (text[i] == '\n')
				{
					sizes.push_back(width);
					width = 0;
					++height;
				} else ++width;
			}

			if (height != 1)
			{
				for (uint32 i = 0; i < sizes.size(); ++i)
				{
					if (width < sizes[i]){width = sizes[i];
					}
				}
			}
			width = std::max<uint32>(width, 1);
		}
	public:
		Text()
		{
			this->charWidth = 0;
			this->charHeight = 0;
		}
		Text(Image* fontImage, uint32 charWidth, uint32 charHeight, const std::string text)
		{
			this->font = fontImage;
			this->charWidth = charWidth;
			this->charHeight = charHeight;
			this->text = text;
			setText(text);
		}
		~Text()
		{
			this->font = nullptr;
		}

		//Pass in image pointer with font loaded to save memory
		void setFont(Image* fontImage, uint32 charWidth, uint32 charHeight)
		{
			font = fontImage;
			this->charWidth = charWidth;
			this->charHeight = charHeight;
			return;
		}
		void setCharSize(uint32 charWidth, uint32 charHeight)
		{
			this->charWidth = charWidth;
			this->charHeight = charHeight;
			return;
		}
		void setPos(uint32 x, uint32 y)
		{
			textImage.setPos(x, y);
			return;
		}
		uint32 getPosX(void){return textImage.getPosX();
		}
		uint32 getPosY(void){return textImage.getPosY();
		}

		void setText(const std::string text, uint32 compX = 0, uint32 compY = 0)
		{
			this->text = text;
			uint32 textWidth, textHeight;
			getTextSize(text, textWidth, textHeight);
			textImage.setSize((textWidth * charWidth) - (compX * textWidth), (textHeight * charHeight) - (compY * textHeight), true);
			uint32 srcX = 0, srcY = 0, dstX = 0, dstY = 0;

			for (uint32 i = 0; i < text.size(); ++i)
			{
				uint8 c = text[i];
				srcX = c * charWidth;

				switch (c)
				{
					case '\n':
						dstX = 0;
						dstY++;
						break;

					default:
						textImage.copy(*font, (dstX * charWidth) - (compX * dstX), (dstY * charHeight) - (compY * dstY), srcX, 0, charWidth, charHeight, true);
						dstX++;
						break;
				}
			}
			return;
		}

		std::string getText(void){return text;
		}
		Image& getTextImage(void){return textImage;
		}
		Image* getFontImage(void){return font;
		}

		uint32 getCharWidth(void){return charWidth;
		}
		uint32 getCharHeight(void){return charHeight;
		}
		uint32 getWidth(void){return textImage.getWidth();
		}
		uint32 getHeight(void){return textImage.getHeight();
		}
	};

	class ConsoleGraphics
	{
		std::vector<uint32> pixels;
		uint32 width, height, startX, startY, consoleWidth, consoleHeight;
		RenderMode renderMode;
		HBITMAP pixelBitmap;
		HDC targetDC, tempDC;
		bool alphaMode;
		uint16 pixelSize;
		bool enableShaders;
		std::vector<void(*)(uint32*, uint32, uint32, uint32, uint32, void*)> shaderList;
		std::vector<void*> shaderDataList;
		std::string title;
		float outputScale = 1.f;
		//"shaderList" function struct
		//Arg 1 = Pointer to current pixel
		//Arg 2 = Current X
		//Arg 3 = Current Y
		//Arg 4 = Number of operations
		//Arg 5 = Extra data
	protected:
		void initialise(void)
		{
			targetDC = GetDC(GetConsoleWindow());
			tempDC = CreateCompatibleDC(targetDC);

			renderMode = RenderMode::BitBlt;
			startX = 0;
			startY = 0;
			pixelSize = 1;
			outputScale = 1.f;

			alphaMode = false;
			enableShaders = false;

			return;
		}

		//Improve this
		void RemoveScrollbar(void)
		{
			ShowScrollBar(GetConsoleWindow(), SB_BOTH, FALSE);
			return;
		}

		void GetWindowSize(HWND w, uint32& width, uint32& height)
		{
			RECT window;
			GetWindowRect(w, &window);

			width = window.right - window.left;
			height = window.bottom - window.top;
			return;
		}

		void SetConsoleSize(uint32 width, uint32 height)
		{
			RECT window;
			GetWindowRect(GetConsoleWindow(), &window);
			SetWindowPos(GetConsoleWindow(), HWND_TOP, window.left, window.top, width, height, SWP_SHOWWINDOW);
			RemoveScrollbar();
			return;
		}

		std::vector<uint32> ResizeDataNearestNeighbor(std::vector<uint32>& pixels, uint32 newWidth, uint32 newHeight)
		{
			std::vector<uint32> data;
			float xScale = (float)width / (float)newWidth, yScale = (float)height / (float)newHeight;
			data.resize(newWidth * newHeight);

			uint32 srcX, srcY;
			for (uint32 y = 0; y < newHeight; ++y)
			{
				for (uint32 x = 0; x < newWidth; ++x)
				{
					srcX = x * xScale;
					srcY = y * yScale;
					data[(y * newWidth) + x] = pixels[(srcY * width) + srcX];
				}
			}

			width = newWidth;
			height = newHeight;
			return data;
		}

		uint32& accessBuffer(uint32 x, uint32 y){return pixels[(y * width) + x];
		}
		uint32& accessBuffer(uint32 index){return pixels[index];
		}

	public:
		ConsoleGraphics()
		{
			initialise();
			GetWindowSize(GetConsoleWindow(), consoleWidth, consoleHeight);
			RemoveScrollbar();
			width = consoleWidth;
			height = consoleHeight;
			pixels.resize(width * height);
		}

		//Creates window closest to specified dimensions.
		//If setSize = true, console window is automatically resized
		//If pixelMode = true
		ConsoleGraphics(uint32 width, uint32 height, bool setSize = false, uint16 pixelSize = 1, bool pixelMode = false)
		{
			initialise();
			this->pixelSize = std::max<uint16>(pixelSize, 1);
			GetWindowSize(GetConsoleWindow(), consoleWidth, consoleHeight);
			this->width = width / (!pixelMode ? 1 : pixelSize);
			this->height = height / (!pixelMode ? 1 : pixelSize);

			if (setSize || pixelMode)
			{
				//Get windows version and add offset to window size
				uint16 windowWidthOffset = 0, windowHeightOffset = 0;
				SetConsoleSize(consoleWidth + windowWidthOffset, consoleHeight + windowHeightOffset);
			}

			pixels.resize(this->width * this->height);
		}

		bool display(void)
		{
			bool returnValue = true;

			if (enableShaders)
			{
				for (uint8 i = 0; i < shaderList.size(); ++i)
				{
					for (uint32 y = 0; y < height; ++y)
					{
						for (uint32 x = 0; x < width; ++x)
						{
							shaderList[i](&pixels[(y * width) + x], width, height, x, y, shaderDataList[i]);
						}
					}
				}
			}

			switch (renderMode)
			{
				default:
				case RenderMode::BitBltInv:
				case RenderMode::BitBlt:
					pixelBitmap = CreateBitmap(width, height, 1, 8 * 4, &pixels[0]);
					if (returnValue = (pixelBitmap == NULL))
					{
						#ifdef CG_DEBUG
							std::cerr << "CGOUT ERROR {this->display()}: CreateBitmap() returned 'NULL' [GetLastError()=" << GetLastError() << "]" << std::endl;
						#endif

						return !returnValue;
					}
					SelectObject(tempDC, pixelBitmap);
					returnValue = StretchBlt(targetDC, startX, startY, consoleWidth * outputScale, consoleHeight * outputScale, tempDC, 0, 0, width, height, renderMode == RenderMode::BitBlt ? SRCCOPY : NOTSRCCOPY);
					if (!returnValue)
					{
						#ifdef CG_DEBUG
							std::cerr << "CGOUT ERROR {this->display()}: StretchBlt() returned 'false' [GetLastError()=" << GetLastError() << "]" << std::endl;
						#endif
						
						return !returnValue;
					}
					DeleteObject(pixelBitmap);
					break;

				case RenderMode::SetPixelInv:
				case RenderMode::SetPixel:
					for (uint32 y = 0; y < consoleHeight; ++y)
					{
						for (uint32 x = 0; x < consoleWidth; ++x)
						{
							uint32 p = _byteswap_ulong(pixels[((y / pixelSize) * width) + (x / pixelSize)]) >> 8;
							SetPixelV(targetDC, x, y, renderMode == RenderMode::SetPixel ? p : p ^ 0x00FFFFFF); //^ 0x00FFFFFF << inverts colours
						}
					}
					break;

				case RenderMode::SetPixelVer:
				case RenderMode::SetPixelVerInv:
					for (uint32 x = 0; x < consoleWidth; ++x)
					{
						for (uint32 y = 0; y < consoleHeight; ++y)
						{
							uint32 p = _byteswap_ulong(pixels[((y / pixelSize) * width) + (x / pixelSize)]) >> 8;
							SetPixelV(targetDC, x, y, renderMode == RenderMode::SetPixelVer ? p : p ^ 0x00FFFFFF);
						}
					}
					break;
			}

			return returnValue;
		}

		void setRenderMode(RenderMode mode)
		{
			renderMode = mode;
			return;
		}
		void enableAlpha(void)
		{
			this->alphaMode = true;
			return;
		}
		void disableAlpha(void)
		{
			this->alphaMode = false;
			return;
		}

		//Clear window with grayscale value
		void clear(uint8 c = 0x00)
		{
			memset(pixels.data(), c, pixels.size() * sizeof(uint32));
			return;
		}

		void setPixel(uint32 x, uint32 y, uint32 rgb)
		{
			if (x < width && y < height){pixels[(y * width) + x] = rgb;
			}
			return;
		}
		void drawPixel(uint32 x, uint32 y, uint32 rgb, uint8 a)
		{
			if (x < width && y < height && alphaMode)
			{
				pixels[(y * width) + x] = blendPixel(pixels[(y * width) + x], rgb, a);
			}
			return;
		}
		uint32* getPixel(uint32 x, uint32 y)
		{
			if (x < width && y < height){return &pixels[(y * width) + x];
			} else return nullptr;
		}
		uint32* accessPixel(uint32 x, uint32 y){return &pixels[(y * width) + x];
		}
		const uint32* getPixelData(void) const {return &pixels[0];
		}

		uint32 getWidth(void){return width;
		}
		uint32 getHeight(void){return height;
		}

		uint32 getConsoleWidth(void){return consoleWidth;
		}
		uint32 getConsoleHeight(void){return consoleHeight;
		}

		uint16 getPixelSize(void){return pixelSize;
		}

		//Pixelizes the output already drawn to the screen (this is a very costly function, only use when nessesary)
		void pixelize(const float ratio)
		{
			if (!(ratio <= 1.f) && ratio < width && ratio < height)
			{
				pixels = ResizeDataNearestNeighbor(pixels, width / ratio, height / ratio);
				pixels = ResizeDataNearestNeighbor(pixels, consoleWidth / pixelSize, consoleHeight / pixelSize);
			}
			return;
		}

		void lineHorizontal(uint32 x, uint32 y, uint32 iterations, uint32 rgb, bool forward = true)
		{
			if (x < width && y < height && ((forward && x + iterations < width) || (!forward && x - iterations < width)))
			{
				for (uint32 i = 0; i < iterations; ++i)
				{
					accessBuffer(forward ? x + i : x - i, y) = rgb;
				}
			}
			else {
				for (uint32 i = 0; i < iterations; ++i)
				{
					if (x + i < width){accessBuffer(forward ? x + i : x - i, y) = rgb;
					}
					else break;
				}
			}
			return;
		}

		void lineVertical(uint32 x, uint32 y, uint32 iterations, uint32 rgb, bool forward = true)
		{
			if (x < width && y < height && ((forward && y + iterations < height) || (!forward && y - iterations < height)))
			{
				for (uint32 i = 0; i < iterations; ++i)
				{
					accessBuffer(x, forward ? y + i : y - i) = rgb;
				}
			}
			else {
				for (uint32 i = 0; i < iterations; ++i)
				{
					if (y + i < height){accessBuffer(x, forward ? y + i : y - i) = rgb;
					} else break;
				}
			}
			return;
		}

		void drawLine(uint32 x0, uint32 y0, uint32 x1, uint32 y1, uint32 rgb, uint8 alpha = 255)
		{
			int32 w, h, l;
			float dx, dy;
			uint32 x = x0, y = y0;

			w = x1 - x0;
			h = y1 - y0;
			l = ceil(sqrtf((w * w) + (h * h)));
			dx = (float)w / (float)l;
			dy = (float)h / (float)l;

			for (uint32 i = 0; i < l; ++i)
			{
				x = x0 + (dx * i);
				y = y0 + (dy * i);
				if (x < width && y < height)
				{
					switch (alpha)
					{
						case 255:
							accessBuffer(x, y) = rgb;
							break;

						default:
							accessBuffer(x, y) = blendPixel(accessBuffer(x, y), rgb, alpha);
							break;
					}
				} else break;
			}
			return;
		}

		void drawRect(uint32 x, uint32 y, uint32 width, uint32 height, uint32 rgb, bool fill = true)
		{
			if (fill)
			{
				for (uint32 dy = y; dy < y + height; ++dy)
				{
					for (uint32 dx = x; dx < x + width; ++dx)
					{
						if (dx < this->width && dy < this->height)
						{
							pixels[(dy * this->width) + dx] = rgb;
						}
					}
				}
			}
			else {
				lineHorizontal(x, y, width, rgb);
				lineVertical(x + width, y, height, rgb);
				lineHorizontal(x + width, y + height, width, rgb, false);
				lineVertical(x, y + height, height, rgb, false);
			}
			return;
		}
		void drawRectA(uint32 x, uint32 y, uint32 width, uint32 height, uint32 rgb, uint8 a)
		{
			for (uint32 dy = y; dy < y + height; ++dy)
			{
				for (uint32 dx = x; dx < x + width; ++dx)
				{
					if (dx < this->width && dy < this->height)
					{
						pixels[(dy * this->width) + dx] = blendPixel(pixels[(dy * this->width) + dx], rgb, a);
					}
				}
			}
			return;
		}

		//Enable Post-Processing shaders
		void enablePPShaders(void)
		{
			enableShaders = true;
			return;
		}
		//Disable Post-Processing shaders
		void disablePPShaders(void)
		{
			enableShaders = false;
			return;
		}
		//Load Post-Processing shaders for ConsoleGraphics to use
		void loadPPShader(void(*funcPtr)(uint32*, uint32, uint32, uint32, uint32, void*), void* shaderData = nullptr)
		{
			shaderList.push_back(funcPtr);
			shaderDataList.push_back(shaderData);
			return;
		}
		//Clear all Post-Processing shaders
		void clearPPShaders(void)
		{
			shaderList.clear();
			shaderDataList.clear();
			return;
		}
		//Draws image to a buffer
		void draw(Image& image)
		{
			for (uint32 y = 0; y < image.getHeight(); ++y)
			{
				for (uint32 x = 0; x < image.getWidth(); ++x)
				{
					uint32 dstX = x + image.getPosX(), dstY = y + image.getPosY();
					if (dstX < width && dstY < height) //Within bounds
					{
						if (image.getPixel(x, y)->second == 255 || !alphaMode) //Can't do alpha or alpha isn't enabled
						{
							pixels[(dstY * width) + dstX] = image.accessPixel(x, y)->first;
						} else if (image.getPixel(x, y)->second != 0) { //Alpha is enabled
							pixels[(dstY * width) + dstX] = blendPixel(pixels[(dstY * width) + dstX], image.accessPixel(x, y)->first, image.accessPixel(x, y)->second);
						}
					} else if (dstY > height - 1){return;
					} else if (dstX > width - 1){break;
					}
				}
			}
			return;
		}
		void draw(Text& text)
		{
			this->draw(text.getTextImage());
			return;
		}
		//drawType = DrawType::Repeat - if width > image.width() or height > image.height(), the image will be tiled
		//drawType = DrawType::Resized - if width > image.width() or height > image.height(), the image will be resampled using nearest neighbor interpolation
		//A more advanced version of the draw function
		void drawEX(Image& image, uint32 srcX, uint32 srcY, uint32 dstX, uint32 dstY, uint32 width, uint32 height, DrawType drawType = DrawType::Repeat, void(*funcPtr)(std::pair<uint32, uint8>*, void*) = nullptr, void* funcData = nullptr)
		{
			uint32 x = srcX, y = srcY, dx = dstX, dy = dstY;

			if (drawType == DrawType::Repeat)
			{
				while (dy < dstY + height && dy < this->height)
				{
					while (dx < dstX + width && dx < this->width)
					{
						if (dx < this->width && dy < this->height) //Within bounds
						{
							auto pixel = image.getPixelData()[(y * image.getWidth()) + x];
							if (funcPtr != nullptr){funcPtr(&pixel, funcData);
							}
							if (pixel.second == 255 || !alphaMode) //Can't do alpha or alpha isn't enabled
							{
								pixels[(dy * this->width) + dx] = pixel.first;//image.getPixel(x, y)->first;
							} else if (pixel.second != 0){ //Alpha is enabled
								pixels[(dy * this->width) + dx] = blendPixel(pixels[(dy * this->width) + dx], pixel.first, pixel.second);
							}
						} else if (dy > this->height - 1){return;
						} else if (dx > this->width - 1){break;
						}
						if (x == image.getWidth() - 1){x = 0;
						}
						else ++x;
						++dx;
					}
					x = srcX;
					dx = dstX;
					if (y == image.getHeight() - 1){y = 0;
					} else ++y;
					++dy;
				}
			} else {
				if (srcX >= image.getWidth()){srcX %= image.getWidth();
				}
				if (srcY >= image.getHeight()){srcY %= image.getHeight();
				}
				float scaleX = (float)(image.getWidth() - srcX) / (float)width, scaleY = (float)(image.getHeight() - srcY) / (float)height;

				while (dy < dstY + height && dy < this->height)
				{
					while (dx < dstX + width && dx < this->width)
					{
						if (dx < this->width && dy < this->height) //Within bounds
						{
							x = srcX + ((dx - dstX) * scaleX);
							x = std::max<int>(std::min<int>(x, image.getWidth()), 0);
							y = srcY + ((dy - dstY) * scaleY);
							y = std::max<int>(std::min<int>(y, image.getHeight()), 0);
							auto pixel = image.getPixelData()[(y * image.getWidth()) + x];
							if (funcPtr != nullptr){funcPtr(&pixel, funcData);
							}
							if (pixel.second == 255 || !alphaMode) //Can't do alpha or alpha isn't enabled
							{
								pixels[(dy * this->width) + dx] = pixel.first;//image.getPixel(x, y)->first;
							} else if (pixel.second != 0){ //Alpha is enabled
								pixels[(dy * this->width) + dx] = blendPixel(pixels[(dy * this->width) + dx], pixel.first, pixel.second);
							}
						} else if (dy > this->height - 1){return;
						} else if (dx > this->width - 1){break;
						}
						++dx;
					}
					dx = dstX;
					++dy;
				}
			}
		}

		void setTitle(const std::string title)
		{
			this->title = title;
			SetConsoleTitleA(this->title.c_str());
			return;
		}
		std::string getTitle(void){return title;
		}

		void setOutputScale(float s)
		{
			outputScale = s;
			return;
		}
		float getOutputScale(void){return outputScale;
		}

		void setOutputPos(uint32 x, uint32 y)
		{
			startX = x;
			startY = y;
			return;
		}
		uint32 getOutputPosX(void){return startX;
		}
		uint32 getOutputPosY(void){return startY;
		}

		void setRenderTarget(HWND hwnd)
		{
			targetDC = GetDC(hwnd);
			tempDC = CreateCompatibleDC(targetDC);
			return;
		}
		void setRenderTarget(HDC hdc)
		{
			targetDC = hdc;
			tempDC = CreateCompatibleDC(targetDC);
			return;
		}
		HWND getRenderTarget(void){return WindowFromDC(targetDC);
		}
	};
};

#endif