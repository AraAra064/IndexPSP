#include <vector>

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

#ifndef __KADA_TOOLS__
#define __KADA_TOOLS__

namespace KadaTools
{

struct MipsCpy
{
	std::vector<uint8> *container, *container2;
	
	long lbu(long adr){
	if (adr & 0x40000000){
		unsigned char value = reinterpret_cast<unsigned char &>((*container)[adr & 0x0FFFFFFF]);
		return value;
	}
	//return *(reinterpret_cast<unsigned char *>(adr));
	return  reinterpret_cast<unsigned char &>((*container2)[adr]);
}

void sb(long toStore, long location){
	location = location & 0x0FFFFFFF;
	if (location >= container->size()){
		container->resize(location + 1);
	}
	//unsigned char *loc = reinterpret_cast<unsigned char *>(location);
	//*loc = toStore & 0xFF;
	unsigned char byte = (toStore & 0xFF);
	(*container)[location] = byte;
}
};

std::vector<uint8> DecodeRLE2(std::vector<uint8> &buff){
	MipsCpy mips;

	std::vector<uint8> result;
	//result.resize(buff.size()*20);

	long a0, a1, a2, a3, v0, v1, t0, t1, t2;
	bool cond;

	a0 = 0;//reinterpret_cast<long>(&(buff[0]));//position in src
	a1 = 0x40000000;//reinterpret_cast<long>(&(result[0]));//position in target
	mips.container = &result;
	mips.container2 = &buff;

	//		move	a2,a0
	a2 = a0;
	//		lbu	a0,0x0(a2)
	a0 = mips.lbu(a2);
	//		move	a3,a1
	a3 = a1;
	//		li	t2,0x60
	t2 = 0x60;
	//		beq	zero,a0,jmp1
	cond = (a0 == 0);
	//		addiu	t0,a2,0x1
	t0 = a2 + 1; if (cond) goto jmp1;
	//jmp7:		seb	v0,a0
jmp7:
	v0 = a0 & 0x80;
	//		bltz	v0,jmp2
	cond = (v0);
	//		andi	v0,a0,0x40
	v0 = a0 & 0x40; if (cond) goto jmp2;
	//		beql	zero,v0,jmp3
	cond = (v0 == 0);
	//		andi	v0,a0,0x20
	v0 = a0 & 0x20; if (cond) goto jmp3;
	//		andi	v0,a0,0xF
	v0 = a0 & 0xF;
	//		andi	v1,a0,0x10
	v1 = a0 & 0x10;
	//		beq	zero,v1,jmp4
	cond = (v1 == 0);
	//		addiu	v0,v0,0x4
	v0 = v0 + 0x4; if (cond) goto jmp4;
	//		lbu	v1,0x1(a2)
	v1 = mips.lbu(a2 + 1);
	//		andi	v0,a0,0xF
	v0 = a0 & 0xF;
	//		sll	v0,v0,0x8
	v0 = v0 << 0x8;
	//		or	v1,v1,v0
	v1 = v1 | v0;
	//		addiu	t0,a2,0x2
	t0 = a2 + 0x2;
	//		addiu	v0,v1,0x4
	v0 = v1 + 0x4;
	//jmp4:		lbu	v1,0x0(t0)
jmp4:
	v1 = mips.lbu(t0);
	//		blez	v0,jmp5
	cond = (v0 == 0);
	//		addiu	t0,t0,0x1
	t0 = t0 + 1; if (cond) goto jmp5;
	//		move	a0,a3
	a0 = a3;
	//		addu	v0,v0,a3
	v0 = v0 + a3;
	//		sb	v1,0x0(a0)
	mips.sb(v1, a0);
	//jmp6:		addiu	a0,a0,0x1
jmp6:
	a0 = a0 + 1;
	//		bnel	v0,a0,jmp6
	cond = (v0 != a0);
	//		sb	v1,0x0(a0)
	mips.sb(v1, a0); if (cond) goto jmp6;
	//		move	a3,a0
	a3 = a0;
	//jmp5:		move	a2,t0
jmp5:
	a2 = t0;
	//jmp10:		lbu	a0,0x0(a2)
jmp10:
	a0 = mips.lbu(a2);
	//		bne	zero,a0,jmp7
	cond = (a0 != 0);
	//		addiu	t0,a2,0x1
	t0 = a2 + 1; if (cond) goto jmp7;
	//jmp1:		lui	v0,0x89E
jmp1:
	v0 = 0x89E << 16;
	//		subu	a0,a3,a1
	a0 = a3 - a1;
	//		sw	t0,-0x44F0(v0)
	//sw(t0, v0 - 0x44F0);//--------------------------------------------------------------
	//		lui	v1,0x89E
	v1 = 0x89E;
	//		lui	v0,0x89E
	v0 = 0x89E;
	//		sw	a0,-0x44F8(v1)
	//sw(a0, v1 - 0x44F8);//----------------------------------------------------------------
	//		jr	ra
	//		sw	a3,-0x44F4(v0)
	//sw(a3, v0 - 0x44F4);//-----------------------------------------------------------------
	result.resize(a0);
	return result;
	//jmp3:		beq	zero,v0,jmp8
jmp3:
	cond = (v0 == 0);
	//		andi	v1,a0,0x1F
	v1 = a0 & 0x1F; if (cond) goto jmp8;
	//		lbu	v1,0x1(a2)
	v1 = mips.lbu(a2 + 1);
	//		andi	v0,a0,0x1F
	v0 = a0 & 0x1F;
	//		sll	v0,v0,0x8
	v0 = v0 << 8;
	//		or	v1,v1,v0
	v1 = v1 | v0;
	//		addiu	t0,a2,0x2
	t0 = a2 + 2;
	//jmp8:		move	a2,a3
jmp8:
	a2 = a3;
	//		blez	v1,jmp5
	cond = (v1 <= 0);
	//		addu	t1,v1,a3
	t1 = v1 + a3; if (cond) goto jmp5;
	//jmp9:		lbu	v0,0x0(t0)
jmp9:
	v0 = mips.lbu(t0);
	//		sb	v0,0x0(a2)
	mips.sb(v0, a2);
	//		addiu	a2,a2,0x1
	a2 = a2 + 1;
	//		bne	a2,t1,jmp9
	cond = (a2 != t1);
	//		addiu	t0,t0,0x1
	t0 = t0 + 1; if (cond) goto jmp9;
	//		move	a3,t1
	a3 = t1;
	//		j	jmp10
	cond = true;
	//		move	a2,t0
	a2 = t0; if (cond) goto jmp10;
	//jmp2:		lbu	v0,0x1(a2)
jmp2:
	v0 = mips.lbu(a2 + 1);
	//		andi	v1,a0,0x1F
	v1 = a0 & 0x1F;
	//		sll	v1,v1,0x8
	v1 = v1 << 8;
	//		ext	a0,a0,0x5,0x2
	a0 = (a0 & 0x60) >> 5;
	//		or	v0,v0,v1
	v0 = v0 | v1;
	//		addiu	v1,a0,0x4
	v1 = a0 + 0x4;
	//		addiu	t0,a2,0x2
	t0 = a2 + 0x2;
	//		blez	v1,jmp11
	cond = (v1 <= 0);
	//		subu	a2,a3,v0
	a2 = a3 - v0; if (cond) goto jmp11;
	//		move	a0,a3
	a0 = a3;
	//		addu	v1,v1,a3
	v1 = v1 + a3;
	//jmp12:		lbu	v0,0x0(a2)
jmp12:
	v0 = mips.lbu(a2);
	//		sb	v0,0x0(a0)
	mips.sb(v0, a0);
	//		addiu	a0,a0,0x1
	a0 = a0 + 1;
	//		bne	v1,a0,jmp12:
	cond = (v1 != a0);
	//		addiu	a2,a2,0x1
	a2 = a2 + 1; if (cond) goto jmp12;
	//		move	a3,a0
	a3 = a0;
	//jmp11:		lbu	v1,0x0(t0)
jmp11:
	v1 = mips.lbu(t0);
	//		andi	v0,v1,0xE0
	v0 = v1 & 0xE0;
	//		bne	t2,v0,jmp5
	cond = (t2 != v0);
	//		andi	v0,v1,0x1F
	v0 = v1 & 0x1F; if (cond) goto jmp5;
	//		blez	v0,jmp11
	cond = (v0 <= 0);
	//		addiu	t0,t0,0x1
	t0 = t0 + 1; if (cond) goto jmp11;
	//		move	v1,a2
	v1 = a2;
	//		addu	a0,a2,v0
	a0 = a2 + v0;
	//jmp13:		lbu	v0,0x0(v1)
jmp13:
	v0 = mips.lbu(v1);
	//		addiu	v1,v1,0x1
	v1 = v1 + 1;
	//		sb	v0,0x0(a3)
	mips.sb(v0, a3);
	//		bne	a0,v1,jmp13
	cond = (a0 != v1);
	//		addiu	a3,a3,0x1
	a3 = a3 + 1; if (cond) goto jmp13;
	//		j	jmp11
	cond = true;
	//		move	a2,v1
	a2 = v1; if (cond) goto jmp11;

	return result;
}

}

#endif
