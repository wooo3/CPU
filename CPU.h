//CPU
#ifndef CCharaCPU_H
#define CCharaCPU_H

#include <stdarg.h>
#include <cstdarg>
#include "..\Training\Training.h"


#define KABE 3.30f

const float THROW_DISTANCE = 0.53;
const float SHORT_DISTANCE = 1.25;
const float LONG_DISTANCE = 2.75;
const float MIDDLE_DISTANCE = (SHORT_DISTANCE + LONG_DISTANCE)/2;

//AI切り替え時間（切り替えるのが早いほど強い）
const int AIchangetime_yowai = 30;
const int AIchangetime_hutuu = 20;
const int AIchangetime_tuyoi = 10;



enum CPU_STATE{
	CPU_STATE_VS_STAND,
	CPU_STATE_VS_CROUCH,
	CPU_STATE_VS_AIR,
	CPU_STATE_VS_PUNISH,
	CPU_STATE_VS_OKI,
	CPU_STATE_VS_GUARD_REVERSAL,
	CPU_STATE_VS_DOWN_REVERSAL,
	CPU_STATE_ALL,
};
enum CPU_DISTANCE{
	CPU_DISTANCE_THROW,
	CPU_DISTANCE_SHORT,
	CPU_DISTANCE_MIDDLE,
	CPU_DISTANCE_LONG,
	CPU_DISTANCE_OUT,		//レンジがい
	CPU_DISTANCE_ALL,
};


enum CPU_MODE{
	CPU_MODE_PVP,	//プレイヤーＶＳプレイヤー
	CPU_MODE_PVC,	//プレイヤーＶＳＣＰＵ
	CPU_MODE_CVC,	//観戦
	CPU_MODE_TRAINING,	//トレモ
	CPU_MODE_ACTEDIT,	//エディット中判別用
};
class CPU_CONTROLL{
public:
	CPU_CONTROLL(){
		cpi_ai = CPU_MODE_PVP;
	};
	static CPU_CONTROLL* GetInst(){	// インスタンスの生成
		static CPU_CONTROLL Inst;
		return &Inst;
	};
	CPU_MODE cpi_ai;	//0で対人戦　1でCPU戦　2でCPU同士
};

#endif