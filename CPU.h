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

//AI�؂�ւ����ԁi�؂�ւ���̂������قǋ����j
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
	CPU_DISTANCE_OUT,		//�����W����
	CPU_DISTANCE_ALL,
};


enum CPU_MODE{
	CPU_MODE_PVP,	//�v���C���[�u�r�v���C���[
	CPU_MODE_PVC,	//�v���C���[�u�r�b�o�t
	CPU_MODE_CVC,	//�ϐ�
	CPU_MODE_TRAINING,	//�g����
	CPU_MODE_ACTEDIT,	//�G�f�B�b�g�����ʗp
};
class CPU_CONTROLL{
public:
	CPU_CONTROLL(){
		cpi_ai = CPU_MODE_PVP;
	};
	static CPU_CONTROLL* GetInst(){	// �C���X�^���X�̐���
		static CPU_CONTROLL Inst;
		return &Inst;
	};
	CPU_MODE cpi_ai;	//0�őΐl��@1��CPU��@2��CPU���m
};

#endif