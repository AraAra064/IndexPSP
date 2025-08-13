#include <windows.h>
#include <fstream>
#include <conio.h>
#include <math.h>
#include <chrono>
#include <deque>
#include <vector>

#ifndef CU_INCLUDE
#ifdef INCREASE_MOUSE_ACCURACY
	#define MOUSE_X_OFFSET 10 //5
	#define MOUSE_Y_OFFSET 32 //20
#else 
	#define MOUSE_X_OFFSET 0
	#define MOUSE_Y_OFFSET 0
#endif
#endif

#ifndef int8
	#define int8 char
	typedef unsigned int8 uint8;
	#define int16 short int
	typedef unsigned int16 uint16;
	#define int32 long int
	typedef unsigned int32 uint32;
	#define int64 long long int
	typedef unsigned int64 uint64;
#endif


/*int* resizeBilinear(int* pixels, int w1, int h1, int w2, int h2) 
{
    int* retval = new int[w2*h2] ;
    int a, b, c, d, x, y, index ;
    float x_ratio = ((float)(w1-1))/w2 ;
    float y_ratio = ((float)(h1-1))/h2 ;
    float x_diff, y_diff, blue, red, green ;
    int offset = 0 ;
    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            x_diff = (x_ratio * j) - x ;
            y_diff = (y_ratio * i) - y ;
            index = (y*w1+x) ;                
            a = pixels[index] ;
            b = pixels[index+1] ;
            c = pixels[index+w1] ;
            d = pixels[index+w1+1] ;

            // blue element
            // Yb = Ab(1-w1)(1-h1) + Bb(w1)(1-h1) + Cb(h1)(1-w1) + Db(wh)
            blue = (a&0xff)*(1-x_diff)*(1-y_diff) + (b&0xff)*(x_diff)*(1-y_diff) +
                   (c&0xff)*(y_diff)*(1-x_diff)   + (d&0xff)*(x_diff*y_diff);

            // green element
            // Yg = Ag(1-w1)(1-h1) + Bg(w1)(1-h1) + Cg(h1)(1-w1) + Dg(wh)
            green = ((a>>8)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>8)&0xff)*(x_diff)*(1-y_diff) +
                    ((c>>8)&0xff)*(y_diff)*(1-x_diff)   + ((d>>8)&0xff)*(x_diff*y_diff);

            // red element
            // Yr = Ar(1-w1)(1-h1) + Br(w1)(1-h1) + Cr(h1)(1-w1) + Dr(wh)
            red = ((a>>16)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>16)&0xff)*(x_diff)*(1-y_diff) +
                  ((c>>16)&0xff)*(y_diff)*(1-x_diff)   + ((d>>16)&0xff)*(x_diff*y_diff);

            retval[offset++] = 
                    0xff000000 | // hardcoded alpha
                    ((((int)red)<<16)&0xff0000) |
                    ((((int)green)<<8)&0xff00) |
                    ((int)blue) ;
        }
    }
    return retval;
} 

int* resizePixels(int* pixels,int w1,int h1,int w2,int h2) 
{
    int* retval = new int[w2*h2] ;
    // EDIT: added +1 to remedy an early rounding problem
    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;
    //int x_ratio = (int)((w1<<16)/w2) ;
    //int y_ratio = (int)((h1<<16)/h2) ;
    int x2, y2 ;
    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            retval[(i*w2)+j] = pixels[(y2*w1)+x2] ;
        }                
    }                
    return retval;
}*/

#ifndef CU_INCLUDE
#define CU_INCLUDE

namespace cu
{
	POINT cursor = {0, 0};
}

void getCursorPos(void)
{
	GetCursorPos(&cu::cursor);
	return;
}

void getCursorRelativeToScreen(void)
{
	const static HWND console_hwnd = GetConsoleWindow();
	static RECT window_rect;
	GetWindowRect(console_hwnd, &window_rect);
	GetCursorPos(&cu::cursor);
	cu::cursor.x -= (window_rect.left + MOUSE_X_OFFSET);
	cu::cursor.y -= (window_rect.top + MOUSE_Y_OFFSET);
	return;
}

void getCursorRelativeToScreenTransformed(uint8 pixelSize) //Use if pixel size is greater than 1
{
	const static HWND console_hwnd = GetConsoleWindow();
	static RECT window_rect;
	GetWindowRect(console_hwnd, &window_rect);
	GetCursorPos(&cu::cursor);
	cu::cursor.x -= (window_rect.left + MOUSE_X_OFFSET);
	cu::cursor.x /= pixelSize;
	cu::cursor.y -= (window_rect.top + MOUSE_Y_OFFSET);
	cu::cursor.y /= pixelSize;
	return;
}

uint64 GetTotalMemory(void) //in bytes
{
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mem);
	return mem.ullTotalPhys;
}

//uint64 GetUsedRAM(void) //???
//{
//	MEMORYSTATUSEX memory;
//	memory.dwLength = sizeof(MEMORYSTATUSEX);
//	GlobalMemoryStatusEx(&memory);
//	return (float)memory.ullTotalPhys/(1024*1024) - memory.ullAvailPhys/(1024*1024);
//}

uint64 GetAvailableMemory(void) //in bytes
{
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mem);
	return mem.ullAvailPhys;
}

bool doesFileExist(std::string fileName) //only works if the program has read access
{
	std::ifstream readFile(fileName.c_str());
	bool exist = readFile.is_open();
	readFile.close();
	return exist;
}

uint64 getFileSize(std::string fileName) //in bytes
{
	std::ifstream readFile(fileName.c_str());
	
	uint64 fileSize = 0;
	if (readFile.is_open())
	{
		readFile.seekg(0, std::ios::end);
		fileSize = (std::ifstream::pos_type)readFile.tellg();
		readFile.seekg(0);
		readFile.close();
	}
	
	return fileSize;
}

std::deque<std::string> listDir(std::string dir)
{
	std::string dirStr = dir;
	if (dirStr[dirStr.size()-1] == '\\'){dirStr += "*.*";
	} else dirStr += "\\*.*";
	
	std::deque<std::string> fileNames;
	HANDLE dirHandle;
	WIN32_FIND_DATA file;
	dirHandle = FindFirstFile(dirStr.c_str(), &file);
	if (dirHandle != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(dirHandle, &file))
		{
			fileNames.push_back(file.cFileName);
		}
		FindClose(dirHandle);
		fileNames.pop_front(); //Removes ".." from the first element of the deque
	}
	return fileNames;
}

class Clock
{
	uint32 time1;
	
	public:
		Clock(){time1 = GetTickCount();
		}
		
		uint32 getElapsedTime(void){return (GetTickCount()-time1);
		}
		float getElapsedTimeAsSeconds(void){return (float)(GetTickCount()-time1)/1000.f;
		}
		uint32 restart(void)
		{
			static uint32 return_value;
			return_value = (GetTickCount()-time1);
			time1 = GetTickCount();
			return return_value;
		}
};

class HighResClock
{
	std::chrono::high_resolution_clock::time_point t1, t2;
	
	public:
		HighResClock(){t1 = std::chrono::high_resolution_clock::now();
		}
		
		uint32 getElapsedTime(void)
		{
			t2 = std::chrono::high_resolution_clock::now();
			return (uint32)std::chrono::duration_cast<std::chrono::duration<double>>(t2-t1).count()*1000;
		}
		double getElapsedTimeAsSeconds(void)
		{
			t2 = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::duration<double>>(t2-t1).count();
		}
		uint32 restart(void)
		{
			static uint32 retVal;
			retVal = getElapsedTime();
			t1 = std::chrono::high_resolution_clock::now();
			return retVal;
		}
};

template<typename type>
type map(type value, type min, type max, type mapped_min, type mapped_max){return mapped_min + (mapped_max - mapped_min) * ((value - min) / (max - min));
}

void clearInput(void)
{
	while (_kbhit()){_getch();
	}
	return;
}

void pause(void)
{
	std::cout<<"Press any key to continue..."<<std::flush;
	clearInput();
	getch();
	return;
}

float degToRad(float theta){return theta / 180.f * 3.14159265f;
}
float radToDeg(float theta){return theta * 180.f / 3.14159265f;
}

float sinx(float x)
{
	static float p;
	p = x / (3.14159265f*2.f);
	p = p - (int)p;
	
	return 20.785 * p * (p-0.5f) * (p-1.f); 
}

float cosx(float x)
{
	return sinx(x+90.f); 
}

float tanx(float x)
{
	return sinx(x) / cosx(x);
}

struct FPVector2D
{
	float x, y;
	
	FPVector2D()
	{
		x = 0.f;
		y = 0.f;
	}
	FPVector2D(float x, float y = 1.f)
	{
		this->x = x;
		this->y = y;
	}
	
	void fromAngle(float theta)
	{
		x = cosf(theta);
		y = sinf(theta);
		return;
	}
	float mag(void){return sqrtf((x*x)+(y*y));
	}
	
	void multiply(float s)
	{
		x *= s;
		y *= s;
		return;
	}
	void multiply(float x, float y)
	{
		this->x *= x;
		this->y *= y;
		return;
	}
	void multiply(FPVector2D vec)
	{
		x *= vec.x;
		y *= vec.y;
		return;
	}
	
	void add(FPVector2D vec)
	{
		x += vec.x;
		y += vec.y;
		return;
	}
	void sub(FPVector2D vec)
	{
		x -= vec.x;
		y -= vec.y;
		return;
	}
};

float calcAngle2D(FPVector2D vecA, FPVector2D vecB, bool deg = true)
{
	float num = (vecA.x*vecB.x)+(vecA.y*vecB.y);
	float den = vecA.mag()*vecB.mag();
	float theta = acosf(num/den);
	if (deg){theta = radToDeg(theta);
	}
	return theta;
}

bool isEqualTo(float a, float b, float e = 0.1f){return fabs(a-b) <= e;
}

float sgn(float n){return n/fabs(n);
}

struct Vector2D
{
	int32 x, y;
	
	Vector2D()
	{
		x = 0;
		y = 0;
	}
	Vector2D(int32 x, int32 y)
	{
		this->x = x;
		this->y = y;
	}
};

struct FPVector3D
{
	float x, y, z;
	
	FPVector3D()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}
	FPVector3D(float x, float y = 1.f, float z = 1.f)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	
	void fromAngle(float a1, float a2)
	{
		x = cosf(a2) * sinf(a1);
		y = sinf(a2);
		z = cosf(a2) * cosf(a1);
		return;
	}
	
	void multiply(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return;
	}
	void multiply(float x, float y, float z)
	{
		this->x *= x;
		this->y *= y;
		this->z *= z;
		return;
	}
	void multiply(FPVector3D vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		return;
	}
	float mag(void){return sqrt((x*x)+(y*y)+(z*z));
	}
};

struct FPVector4D
{
	float x, y, z, w;
	
	FPVector4D()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
		w = 0.f;
	}
	FPVector4D(float x, float y = 1.f, float z = 1.f, float w = 1.f)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	
	void multiply(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return;
	}
	void multiply(float x, float y, float z, float w)
	{
		this->x *= x;
		this->y *= y;
		this->z *= z;
		this->w *= w;
		return;
	}
	void multiply(FPVector4D vec)
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		w *= vec.w;
		return;
	}
	float mag(void){return sqrt((x*x)+(y*y)+(z*z)+(w*w));
	}
};

class FPVectorND
{
	float *vecElement;
	uint32 dimentions;
};

//|a, b|
//|c, d|
struct FPMatrix2x2
{
	float a, b, c, d;
	
	FPMatrix2x2()
	{
		a = 1.f;
		b = 0.f;
		c = 0.f;
		d = 1.f;
	}
	FPMatrix2x2(float a, float b, float c, float d)
	{
		this->a = a;
		this->b = b;
		this->c = c;
		this->d = d;
	}
	
	float det(void){return (a*d)-(b*c);
	}
	FPMatrix2x2 adj(void){return FPMatrix2x2(d, -b, -c, a);
	}
	void identity(void)
	{
		a = 1.f;
		b = 0.f;
		c = 0.f;
		d = 1.f;
		return;
	}
	
	void multiply(float n)
	{
		a *= n;
		b *= n;
		c *= n;
		d *= n;
		return;
	}
	FPVector2D multiply(FPVector2D vec)
	{
		FPVector2D v;
		v.x = (vec.x*a)+(vec.y*b);
		v.y = (vec.x*c)+(vec.y*d);
		return v;
	}
	
	void inv(void)
	{
		float _d = det();
		*this = adj();
		
		a /= _d;
		b /= _d;
		c /= _d;
		d /= _d;
		return;
	}
	
	//Degrees
	void rotateDeg(float theta)
	{
		a = cosf(degToRad(theta));
		b = -sinf(degToRad(theta));
		c = -b;
		d = a;
		return;
	}
	//Radians
	void rotateRad(float theta)
	{
		a = cosf(theta);
		b = -sinf(theta);
		c = -b;
		d = a;
		return;
	}
};

//|a11, a12, a13|
//|a21, a22, a23|
//|a31, a32, a33|
struct FPMatrix3x3
{
	//    a[y][x]
	float a[3][3];
	
	protected:
		float det2x2(float a, float b, float c, float d){return (a*d)-(b*c);
		}
		
		void multMat(float *matA, float *matB, unsigned int w, unsigned int h)
		{
			float matC[9];
			for (unsigned int y = 0; y < h; ++y)
			{
				for (unsigned int x = 0; x < w; ++x)
				{
					float _i = 0.f;
					
					for (unsigned int i = 0; i+x < w && i+y < h; ++i)
					{
						_i += matA[(y*w)+(i)]*matB[(i*w)+x];
					}
					matC[(y*w)+x] = _i;
				}
			}
			for (unsigned int i = 0; i < 9; ++i){matA[i] = matC[i];
			}
		}
		
	public:
		FPMatrix3x3()
		{
			//|1, 0, 0|
			//|0, 1, 0|
			//|0, 0, 1|
			for (int y = 0; y < 3; ++y)
			{
				for (int x = 0; x < 3; ++x)
				{
					a[y][x] = (x == y ? 1.f : 0.f);
				}
			}
		}
		FPMatrix3x3(float a11, float a12, float a13, float a21, float a22, float a23, float a31, float a32, float a33)
		{
			a[0][0] = a11;
			a[0][1] = a12;
			a[0][2] = a13;
			a[1][0] = a21;
			a[1][1] = a22;
			a[1][2] = a23;
			a[2][0] = a31;
			a[2][1] = a32;
			a[2][2] = a33;
		}
		
		void identity(void)
		{
			for (int y = 0; y < 3; ++y)
			{
				for (int x = 0; x < 3; ++x)
				{
					a[y][x] = (x == y ? 1.f : 0.f);
				}
			}
			return;
		}
		
		float det(void)
		{
			float a11, a12, a13;
			
			a11 = det2x2(a[1][1], a[1][2], a[2][1], a[2][2]);
			a11 *= a[0][0];
			a12 = det2x2(a[1][0], a[1][2], a[2][0], a[2][2]);
			a12 *= -a[0][1];
			a13 = det2x2(a[1][0], a[1][1], a[2][0], a[2][1]);
			a13 *= a[0][2];
			
			return a11+a12+a13;
		}
		
		//|a11, a12, a13|    |a11, a21, a31|
		//|a21. a22. a23| -> |a12, a22, a32|
		//|a31, a32, a33|    |a13, a23, a33|
		void transpose(void)
		{
			std::swap(a[0][1], a[1][0]);
			std::swap(a[0][2], a[2][0]);
			std::swap(a[1][2], a[2][1]);
			return;
		}
		
		bool inv(void)
		{
			float d;
			if ((d = det()) != 0.f)
			{
				d = 1.f/d;
				transpose();
				
				float a11, a12, a13, a21, a22, a23, a31, a32, a33;
				a11 = det2x2(a[1][1], a[1][2], a[2][1], a[2][2]);
				a12 = -det2x2(a[1][0], a[1][2], a[2][0], a[2][2]);
				a13 = det2x2(a[1][0], a[1][1], a[2][0], a[2][1]);
				
				a21 = -det2x2(a[0][1], a[0][2], a[2][1], a[2][2]);
				a22 = det2x2(a[0][0], a[0][2], a[2][0], a[2][2]);
				a23 = -det2x2(a[0][0], a[0][1], a[2][0], a[2][1]);
				
				a31 = det2x2(a[0][1], a[0][2], a[1][1], a[1][2]);
				a32 = -det2x2(a[0][0], a[0][2], a[1][0], a[1][2]);
				a33 = det2x2(a[0][0], a[0][1], a[1][0], a[1][1]);
				
				a[0][0] = a11;
				a[0][1] = a12;
				a[0][2] = a13;
				
				a[1][0] = a21;
				a[1][1] = a22;
				a[1][2] = a23;
				
				a[2][0] = a31;
				a[2][1] = a32;
				a[2][2] = a33;
				
				multiply(d);
				return true;
			} else return false;
		}
		
		FPMatrix3x3 adj(void);
		
		void translate(float x, float y)
		{
			a[0][2] += x;
			a[1][2] += y;
			return;
		}
		
		void skew(float x, float y)
		{
			a[0][1] = x;
			a[1][0] = y;
			return;
		}
		void skewRad(float xRad, float yRad)
		{
			a[0][1] = tanf(xRad);
			a[1][0] = tanf(yRad);
			return;
		}
		void skewDeg(float xDeg, float yDeg)
		{
			a[0][1] = tanf(degToRad(xDeg));
			a[1][0] = tanf(degToRad(yDeg));
			return;
		}
		
		void rotateDeg(float theta)
		{
			a[0][0] = cosf(degToRad(theta));
			a[0][1] = -sinf(degToRad(theta));
			a[1][0] = -a[0][1];
			a[1][1] = a[0][0];
			return;
		}
		void rotateRad(float theta)
		{
			a[0][0] = cosf(theta);
			a[0][1] = -sinf(theta);
			a[1][0] = -a[0][1];
			a[1][1] = a[0][0];
			return;
		}
		
		void multiply(float s)
		{
			for (int y = 0; y < 3; ++y)
			{
				for (int x = 0; x < 3; ++x)
				{
					a[y][x] *= s;
				}
			}
			return;
		}
		
		FPVector3D multiply(FPVector3D vec)
		{
			FPVector3D v;
			v.x = (vec.x*a[0][0])+(vec.y*a[0][1])+(vec.z*a[0][2]);
			v.y = (vec.x*a[1][0])+(vec.y*a[1][1])+(vec.z*a[1][2]);
			v.z = (vec.x*a[2][0])+(vec.y*a[2][1])+(vec.z*a[2][2]);
			return v;
		}
		
		void multiply(FPMatrix3x3 &mat);
};
//Will be implemented later... eventually
void FPMatrix3x3::multiply(FPMatrix3x3 &mat)
{
	float matA2D[9], matB2D[9];
	for (unsigned char y = 0; y < 3; ++y)
	{
		for (unsigned char x = 0; x < 3; ++x)
		{
			matA2D[(y*3)+x] = this->a[y][x];
		}
	}
	for (unsigned char y = 0; y < 3; ++y)
	{
		for (unsigned char x = 0; x < 3; ++x)
		{
			matB2D[(y*3)+x] = mat.a[y][x];
		}
	}
	multMat(&matA2D[0], &matB2D[0], 3, 3);
	for (unsigned char y = 0; y < 3; ++y)
	{
		for (unsigned char x = 0; x < 3; ++x)
		{
			this->a[y][x] = matA2D[(y*3)+x];
		}
	}
	return;
}

//I'll do it later... hopefully
struct FPMatrix4x4
{
	float a[4][4];
	
	protected:
		float det2x2(float a, float b, float c, float d){return (a*d)-(b*c);
		}
		
		void multMat(float **matA, float **matB, unsigned int w, unsigned int h)
		{
			for (unsigned int y = 0; y < h; ++y)
			{
				for (unsigned int x = 0; x < w; ++x)
				{
					float _i = 0.f;
					
					for (unsigned int i = 0; i < w && i < h; ++i)
					{
						_i += matA[y][x+i]*matB[y+i][x];
					}
					matA[y][x] = _i;
				}
			}
		}
		
	public:
		FPMatrix4x4()
		{
			//|1, 0, 0, 0|
			//|0, 1, 0, 0|
			//|0, 0, 1, 0|
			//|0, 0, 0, 1|
			for (int y = 0; y < 4; ++y)
			{
				for (int x = 0; x < 4; ++x)
				{
					a[y][x] = (x == y ? 1.f : 0.f);
				}
			}
		}
		
		//Eventually...
		void projMat(float degFOV, float near, float far, float aspectRatio)
		{
//			a[0][0] = aspectRatio/tanf(degToRad(degFOV)/2.f);
//			a[1][1] = 1.f/tanf(degToRad(degFOV)/2.f);
//			a[2][2] = far/(far-near);
//			a[2][3] = 1.f;
//			a[3][2] = -(far*near)/(far-near);
			return;
		}
		
		void multiply(float n)
		{
			for (unsigned int y = 0; y < 4; ++y)
			{
				for (unsigned int x = 0; x < 4; ++x)
				{
					a[y][x] *= n;
				}
			}
			return;
		}
		void multiply(FPMatrix4x4 &mat);
		FPVector4D multiply(FPVector4D vec)
		{
			FPVector4D v;
			v.x = (vec.x*a[0][0])+(vec.y*a[0][1])+(vec.z*a[0][2])+(vec.w*a[0][3]);
			v.y = (vec.x*a[1][0])+(vec.y*a[1][1])+(vec.z*a[1][2])+(vec.w*a[1][3]);
			v.z = (vec.x*a[2][0])+(vec.y*a[2][1])+(vec.z*a[2][2])+(vec.w*a[2][3]);
			v.w = (vec.x*a[3][0])+(vec.y*a[3][1])+(vec.z*a[3][2])+(vec.w*a[3][3]);
			return v;
		}
};

void FPMatrix4x4::multiply(FPMatrix4x4 &mat)
{
	multMat((float**)&this->a[0][0], (float**)&mat.a[0][0], 4, 4);
	return;
}

class Keyboard
{
	unsigned int maxTime = 100;
	std::vector<bool> keyStates;
	std::vector<unsigned int> keyTimes;
	
	protected:
		void setKeyBuff(void)
		{
			for (unsigned char i = 0; i < keyStates.size(); ++i)
			{
				keyStates[i] = (GetAsyncKeyState(i) & 0x8000);
			}
			return;
		}
	public:
		Keyboard()
		{
			keyStates.resize(0xFF, false);
			keyTimes.resize(0xFF, 0);
		}
		
		bool isKeyDown(unsigned char key, bool hold = false)
		{
			bool b = keyTimes[key] >= maxTime;
			if (b && !hold){keyTimes[key] = 0;
			}
			return b;
		}
		unsigned int getKeyDownTime(unsigned char key){return keyTimes[key];
		}
		unsigned int getMaxDownTime(void){return maxTime;
		}
		void setMaxDownTime(unsigned int t)
		{
			maxTime = t;
			return;
		}
		
		void update(float deltaTime)
		{
			unsigned int t = deltaTime*1000.f;
			for (unsigned char i = 0; i < keyStates.size(); ++i)
			{
				if (GetAsyncKeyState(i) || keyStates[i]){keyTimes[i] += t;
				} else keyTimes[i] = 0;
			}
			setKeyBuff();
			return;
		}
		void update(unsigned int deltaTime)
		{
			for (unsigned char i = 0; i < keyStates.size(); ++i)
			{
				if (GetAsyncKeyState(i) || keyStates[i]){keyTimes[i] += deltaTime;
				} else keyTimes[i] = 0;
			}
			setKeyBuff();
			return;
		}
};

#endif
