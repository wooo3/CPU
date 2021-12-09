#include "CPU.h"
#include "..\Character\Chara.h"
#include "..\Option\Option.h"
#include "..\game.h"	//�L�����N�^�[�l�[�����g����悤�ɂ���


CPU_AI::CPU_AI(){
	Reset();
}

void CPU_AI::Reset(){
	me = NULL;
	enemy = NULL;
	kyori = 0;
	ai_enemy_jump = ai_change = false;
	ai_num = advanceAi = ai_change = ai_count = 0;
	syokika = false;
	AI_ENABLE = false;

	ai_change_1F = false;

	guard_success_count = 0;

	state = CPU_STATE_VS_STAND;
	distance = CPU_DISTANCE_SHORT;

	wait_flg = command_flg = repeat_flg = attack_hit_past_1F = false;

	trainingdata = NULL;

	ai_oh2[0] = ai_oh2[1] = ai_oh2[2];

	for( int i=0 ; i<ALL_CPU_PLAYER_ANALYSIS ; i++ ){
		aiAnalysis[i];
	}

	ai_seq.pack.clear();
}

bool CPU_AI::strinc( string str1, string str2 ){
	if( str1.find( str2 ) != string::npos ){
		return true;
	}
	return false;
}


//AI��csv�����[�h
void CPU_AI::DataLoad( const char* propertydata, istringstream* pIss ){
	Reset();

	istringstream iss;
	if( !pIss ){	//���ڕʌ̃t�@�C������ǂݍ��݁@�A�[�J�C�u���g��Ȃ���
		LLog::GetInstance()->SetFileName( propertydata );
		ifstream ifs( propertydata );
		if( !ifs ){
			MessageBox( 0, 0, propertydata, 0 );
			return;
		}
		ostringstream oss;
		string str;

		//������X�g���[���Ƀt�@�C���̓��e���o��
		oss << ifs.rdbuf();
		str = oss.str();

		iss = istringstream( str );	// create string stream of memory contents	// NOTE: this ends up copying the buffer!!!
	}

	string str;
	vector<vector<string>> table;
	istringstream* pUseIss = &iss;
	if( pIss ){
		pUseIss = pIss;
	}
	while( getline( *pUseIss, str ) ){	//��s���������ǂݍ��݁A��؂蕶���ŕ����ă��X�g�ɒǉ�����

		str.erase( str.find_last_not_of("\r")+1 );	//���s���ލ폜

		// �t�@�C������ǂݍ��񂾂P�s�̕��������؂蕶���ŕ����ă��X�g�ɒǉ�����
		vector<string> record;              // �P�s���̕�����̃��X�g
		istringstream streambuffer( str ); // ������X�g���[��
		string token;                       // �P�Z�����̕�����
		while( getline( streambuffer, token, ',' ) ){
			// �P�Z�����̕���������X�g�ɒǉ�����
			record.push_back( token );
		}
		// �P�s���̕�������o�͈����̃��X�g�ɒǉ�����
		table.push_back( record );
	}

	for( int i=1 ; i<table.size() ; i++ ){	//��s�ڂ͐����Ȃ̂Ŕ�΂�
		if( table[i].empty() || ( table[i].size() < 3 ) ){
			continue;
		}

		if( !table[i][1].empty() || !table[i][2].empty() ){	//�i�G�̏�ԁ��G�Ƃ̋����j����̂Ƃ��͖���
			AI_PARTS tmp;
			string search_buf;	//�ꎞ�i�[�p
			string search_str;	//�����񌟍��p
			unsigned int loc;	//�����񌟍��p
			int count_inc = 1;


			//���
			search_buf = table[i][COUNT_INC];
			for( int j=0 ; j<CPU_STATE_ALL ; j++ ){
				tmp.state = CPU_STATE_VS_STAND;
				if(j == 0)search_str = "STAND";
				if(j == 1)search_str = "CROUCH";
				if(j == 2)search_str = "AIR";
				if(j == 3)search_str = "PUNISH";
				if(j == 4)search_str = "OKI";
				if(j == 5)search_str = "GUARD_REVERSAL";
				if(j == 6)search_str = "DOWN_REVERSAL";
				if( search_buf == search_str ){
					tmp.state = (CPU_STATE)j;
					break;
				}
			}

			//����
			search_buf = table[i][COUNT_INC];
			for( int j=0 ; j<CPU_DISTANCE_ALL ; j++ ){
				tmp.distance = CPU_DISTANCE_THROW;
				if(j == 0)search_str = "THROW";
				if(j == 1)search_str = "SHORT";
				if(j == 2)search_str = "MIDDLE";
				if(j == 3)search_str = "LONG";
				if(j == 4)search_str = "OUT";
				if( search_buf == search_str ){
					tmp.distance = (CPU_DISTANCE)j;
					break;
				}
			}
		
			//�󔒂̃Z�����o�Ă���܂ŌJ��Ԃ�
			for( int j=0 ; j<PACK_MAX ; j++ ){
				search_buf = table[i][COUNT_INC];
				if( !search_buf.empty() ){
					tmp.parts.push_back( search_buf );
				}else{
					break;
				}
			}


			//�ǉ�
			ai_seq.pack.push_back( tmp );
		}
	}
}

void CPU_AI::DataLoadFromFileInMemory( char propertydata[64], CArchive* pArchive ){
	if( !pArchive ){
		MessageBox( 0, 0, "�A�[�J�C�u�t�@�C�����ǂݍ��܂�Ă��܂���", 0 );
		return;
	}

	int inMemoryFileSize = 0;
	char* inMemoryFilePointer = NULL;
	pArchive->GetFileSizeAndPointer( propertydata, &inMemoryFileSize, &inMemoryFilePointer );

	string s( inMemoryFilePointer, ( inMemoryFilePointer + inMemoryFileSize ) );
	istringstream iss( s );

	DataLoad( propertydata, &iss );
}




//��Փx�̌���
//AI�J�n�҂����ԁ@PACK��r���Ŏ~�߂闦�@���������E�K�[�h�E�󂯐g�m��

//���������
//�������łȂ����PACK�I��			
//���n��҂@�U���{�^�������_��	

//PACK�I�����߂�PACK_END_�����ɓ��ꂵ��������������





//��ԃZ�b�g
void CPU_AI::AIStatusSet(){
	state = CPU_STATE_VS_STAND;
	if( enemy->crouch_flg ){
		state = CPU_STATE_VS_CROUCH;
	}
	if( enemy->air_flg && !enemy->damage_flg && ( enemy->dy < 13 || enemy->vecPos.y < 320 ) ){
		state = CPU_STATE_VS_AIR;
	}
	//�N���U��
	if(	enemy->skinmesh.motion_name == "down_faceup" ||
		enemy->skinmesh.motion_name == "down_facedown" ){
			if( enemy->skinmesh.allframe - enemy->skinmesh.meshtimer < 20 ){
				state = CPU_STATE_VS_OKI;
			}
	}

	//���o�T
	if( me->reversal ){
		if( me->skinmesh.motion_name_old == "down_faceup" || me->skinmesh.motion_name_old == "down_facedown" ){
			//�_�E�����o�[�T��
			state = CPU_STATE_VS_DOWN_REVERSAL;
		}else{
			//�K�[�h���o�[�T��
			state = CPU_STATE_VS_GUARD_REVERSAL;
		}
	}
}

//�����Z�b�g
void CPU_AI::AIDistanceSet(){
	float kyori = abs( me->vecPos.x - enemy->vecPos.x );
	
	if( kyori < me->charaproperty.ai_range[0] )distance = CPU_DISTANCE_THROW;				//��������
	if( kyori >= me->charaproperty.ai_range[0] && kyori < me->charaproperty.ai_range[1] )distance = CPU_DISTANCE_SHORT;		//���ߋ���
	if( kyori >= me->charaproperty.ai_range[1] && kyori < me->charaproperty.ai_range[2] )distance = CPU_DISTANCE_MIDDLE;	//������
	if( kyori >= me->charaproperty.ai_range[2] && kyori < me->charaproperty.ai_range[3] )distance = CPU_DISTANCE_LONG;		//������
	if( kyori >= me->charaproperty.ai_range[3] )distance = CPU_DISTANCE_OUT;												//�����W�O
}

//pack�Z�b�g�@������ԂƋ����̒����烉���_���őI��Őݒ肷��
void CPU_AI::AIPackSet(){
	AI_PARTS* parts_pointer_array[64];
	for( int i=0; i<64 ; i++ ){
		parts_pointer_array[i] = NULL;
	}

	int push_count = 0;
	for( int i=0 ; i<ai_seq.pack.size() ; i++ ){
		if( state == ai_seq.pack[i].state && distance == ai_seq.pack[i].distance ){	//��ԂƋ�����������
			parts_pointer_array[push_count] = &ai_seq.pack[i];
			push_count++;
		}
	}

	if( push_count == 0 ){
		AIReset();
		return;
	}

	int ai_rand = rand_n( push_count );		//��v�����S�p�b�N���̃����_��
	use_pack = *parts_pointer_array[ai_rand];

}

//AI�X�^�[�g�@��Փx�Ńf�B���C����������悤�ɂ��Ă���
void CPU_AI::AiStart(){
	

	////��Փx����
	//int dif_rand = rand()%50;
	//int dif_rand2 = (4-GameOption::GetInst()->difficulty)+1;
	//if(dif_rand % dif_rand2 != 0){
	//	AiSet("5","W5");
	//	return;
	//}

	//�G�_�E�����̓X�^�[�g���Ȃ��H
	if(	enemy->skinmesh.motion_name == "down_faceup" ||
		enemy->skinmesh.motion_name == "down_facedown" ){
			//return;
	}
	//�����_�E�������X�^�[�g���Ȃ�
	if(	me->skinmesh.motion_name == "down_faceup" ||
		me->skinmesh.motion_name == "down_facedown" ){
			return;
	}


	AIStatusSet();
	AIDistanceSet();


	/*int dif_rand = ((4-GameOption::GetInst()->optionitem.difficulty)+1)*5;
	int dif_rand2 = rand()%dif_rand;
	if(dif_rand2 < 9){*/
		AIPackSet();
		advanceAi = 0;
		attack_hit_past_1F = false;
	//}

}

void CPU_AI::AIReset(){
	use_pack = AI_PARTS();

}

void CPU_AI::AiProcess(){
	AI_ENABLE = true;

	//if( GameOption::GetInst()->optionitem.difficulty == 0 ){
	if (true) {	//�{���͑S�L����AI�V�[�g��ݒ肵�Ȃ��Ƃ����Ȃ�
		OH2_AI();	//YUTORI
		return;
	}

	if( !syokika ){	//������
		AIReset();
		syokika = true;
	}

	//�L�[�{�[�h�߂�
	me->button->press.flag[LEVER_LEFT] = false;
	me->button->press.flag[LEVER_RIGHT] = false;
	me->button->press.flag[LEVER_UP] = false;
	me->button->press.flag[LEVER_DOWN] = false;
	me->button->press.flag[BUTTON_ATTACK1] = false;
	me->button->press.flag[BUTTON_ATTACK2] = false;
	me->button->press.flag[BUTTON_ATTACK3] = false;
	me->button->press.flag[BUTTON_ATTACK4] = false;
	me->button->press.flag[BUTTON_ATTACK5] = false;
	me->button->press.flag[BUTTON_ATTACK6] = false;

	me->frontpush = false;
	me->backpush = false;

	me->button->push.flag[BUTTON_ATTACK1] = false;
	me->button->push.flag[BUTTON_ATTACK2] = false;
	me->button->push.flag[BUTTON_ATTACK3] = false;
	me->button->push.flag[BUTTON_ATTACK4] = false;
	me->button->push.flag[BUTTON_ATTACK5] = false;
	me->button->push.flag[BUTTON_ATTACK6] = false;
	
	//�O��̐ݒ�
	if( me->direction == DIRECTION::DIRECTION_LEFT ){
		me->frontpress = me->button->press.flag[LEVER_RIGHT];
		me->backpress = me->button->press.flag[LEVER_LEFT];
	}else{
		me->frontpress = me->button->press.flag[LEVER_LEFT];
		me->backpress = me->button->press.flag[LEVER_RIGHT];
	}

	wait_flg = false;
	command_flg = false;
	repeat_flg = false;

	bool motion_end = me->skinmesh.endmotion || ( me->skinmesh.meshtimer >= me->skinmesh.allframe-1 );
	bool attack_done_set_flg = false;
	bool move_push_done_set_flg = false;	//�\���L�[�������t���O

	if( ai_change_1F ){
		ai_change_1F = false;
		return;
	}

	//�_���[�W���A�Ń��Z�b�g
	/*if( (me->damage_flg || me->guard_flg) && me->skinmesh.endmotion && me->knockback_time <= 0 && !me->down_flg ){
		AiStart();
	}*/
	if( me->reversal ){
		AiStart();
	}

	//use_pack�ɉ����K�p����ĂȂ���΃X�^�[�g
	if( use_pack.parts.size() <= 0 ){
		AiStart();
		return;
	}
	bool ai_end = false;
	if( use_pack.parts.size() <= advanceAi ){
		ai_end = true;
	}



	//��������
	if( me->throwed && me->skinmesh.meshtimer == 0 && !me->attack_flg ){
		int dif_rand = ((4-GameOption::GetInst()->optionitem.difficulty)+1)*5;
		int dif_rand2 = rand() % dif_rand;
		if( dif_rand2 < 9 ){
			//������������
		}
		return;
	}

	//GC
	DummyGuardCancel( true );

	//�󂯐g��
	DummyDamage( true );


	/*if(me->skinmesh.motion_name == "grap" || me->skinmesh.motion_name == "dash" || me->skinmesh.motion_name == "backstep"){
		AIReset();
		return;
	}*/

	////�_���[�W��
	//if((me->damage_flg && me->skinmesh.meshtimer == 1) || me->down_flg){
	//	int rand1_5 = rand()%30+5;
	//	char charR[4];
	//	strcpy_s(charR,"R");
	//	char randstr[3];
	//	sprintf(randstr, "%d", rand1_5);
	//	strcat_s(charR, randstr);

	//	string char5R;
	//	char5R = "5,";
	//	char5R += charR;
	//	//AiSet(char5R);

	//	//�N���オ��ɏd�˂�ꂽ���̃K�[�h
	//	if(enemy->attack_hitbox_appear){
	//		if(me->direction == DIRECTION::DIRECTION_LEFT){
	//			me->button->press.flag[LEVER_RIGHT] = me->frontpress;
	//			me->button->press.flag[LEVER_LEFT] = me->backpress;
	//		}else{
	//			me->button->press.flag[LEVER_LEFT] = me->frontpress;
	//			me->button->press.flag[LEVER_RIGHT] = me->backpress;
	//		}
	//	}
	//	
	//	return;
	//}



	if( enemy->hitstop_time > 0 )return;


	if( ai_end ){	//ai�I����
		if( me->skinmesh.motion_name != "stand_wait" ) return;	//ai�I�����A������Ԃɖ߂�܂�AI����

		AiStart();
		advanceAi = 0;
		return;
	}


	string* parts = &use_pack.parts[advanceAi];		//���݂̃p�[�c
	//�֐��������I�ȗ��p�@2��܂Ō��Ă���
	string* function = NULL;
	if( advanceAi+1 < use_pack.parts.size() ){
		function = &use_pack.parts[advanceAi+1];	//�֐�
	}
	string* argument = NULL;
	int iArg = -1;
	if( advanceAi+2 < use_pack.parts.size() ){
		argument = &use_pack.parts[advanceAi+2];	//����
		iArg = atol( argument->c_str() );
	}


	//�J��Ԃ��@REPEAT�@(�ړ��Ƒҋ@�ł����g��Ȃ�)
	if( function != NULL && *function == "REPEAT" ){
		iArg--;			//���s�[�g�񐔂��f�N�������g
		if( iArg < 0 ){
			//���̍s����
			advanceAi+=3;
			repeat_flg = false;
		}else{
			char cArg[64];
			sprintf( cArg, "%d", iArg );

			*argument = cArg;
			repeat_flg = true;
		}
	}
	//�����̊ԂŃ����_���@REPEAT_RANDOM
	if( function != NULL && *function == "REPEAT_RANDOM" ){
		int rnd = rand() % iArg;
		char cArg[64];
		sprintf( cArg, "%d", rnd );

		*function = "REPEAT";	//REPEAT_RANDOM�ŗ����������REPEAT�ɂȂ�
		*argument = cArg;
		repeat_flg = true;
	}

	//�ډ������^�C�~���O�����킹�������p
	if( function != NULL && *function == "NEXT_PARTS_WAIT" ){
		char cArg[64];
		sprintf( cArg, "%d", iArg );

		*function = "NEXT_PARTS_REPEAT";
		*argument = cArg;
		repeat_flg = true;
	}
	if( function != NULL && *function == "NEXT_PARTS_REPEAT" ){
		iArg--;			//���s�[�g�񐔂��f�N�������g
		if( iArg < 0 ){
			//���̍s����
			advanceAi+=3;
			repeat_flg = false;
			parts = &use_pack.parts[advanceAi];		//���݂̃p�[�c
		}else{
			char cArg[64];
			sprintf( cArg, "%d", iArg );

			*argument = cArg;
			repeat_flg = true;
		}
	}


	if( !wait_flg && !command_flg ){
		if( !strinc( *parts, "COMMAND_" ) ){
			//�u6c�v��u2b�v���A�����ł��󂯕t����悤��
			if( strinc( *parts, "2" ) )me->button->press.flag[LEVER_DOWN]	= true;
			if( *parts == "2P" )me->button->push.flag[LEVER_DOWN]			= true;
			else me->button->push.flag[LEVER_DOWN]							= false;

			if( strinc( *parts, "6" ) )me->frontpress						= true;
			if( *parts == "6P" )me->frontpush								= true;
			else me->frontpush												= false;

			if( strinc( *parts, "8" ) )me->button->press.flag[LEVER_UP]		= true;
			if( *parts == "8P" )me->button->push.flag[LEVER_UP]				= true;
			else me->button->push.flag[LEVER_UP]							= false;

			if( strinc( *parts, "4" ) )me->backpress						= true;
			if( *parts == "4P" )me->backpush								= true;
			else me->backpush												= false;

			if( strinc( *parts, "1" ) ){
				me->button->press.flag[LEVER_DOWN]		= true;
				me->backpress							= true;
			}
			if( strinc( *parts, "3" ) ){
				me->button->press.flag[LEVER_DOWN]		= true;
				me->frontpress							= true;
			}
			if( strinc( *parts, "7" ) ){
				me->button->press.flag[LEVER_UP]		= true;
				me->backpress							= true;
			}
			if( strinc( *parts, "9" ) ){
				me->button->press.flag[LEVER_UP]		= true;
				me->frontpress							= true;
			}

			if( strinc( *parts, "a" ) )me->button->push.flag[BUTTON_ATTACK1]		= true;
			if( strinc( *parts, "b" ) )me->button->push.flag[BUTTON_ATTACK2]		= true;
			if( strinc( *parts, "c" ) )me->button->push.flag[BUTTON_ATTACK3]		= true;
			if( strinc( *parts, "d" ) )me->button->push.flag[BUTTON_ATTACK4]		= true;
			if( strinc( *parts, "e" ) )me->button->push.flag[BUTTON_ATTACK5]		= true;
			if( strinc( *parts, "f" ) )me->button->push.flag[BUTTON_ATTACK6]		= true;
			if( !strinc( *parts, "COMMAND_" ) ){	//�R�}���h�łȂ��Ƃ�����i�R�}���h��ATTACK_RAND�͕ʏ����̂��߁j
				if( strinc( *parts, "ATTACK_RAND" ) ){		//�U�������_��
					int a_rand = rand()%6;
					me->button->push.flag[BUTTON_ATTACK1 + a_rand]		= true;
				}
			}

			if( *parts == "2P" ||
				*parts == "4P" ||
				*parts == "6P" ||
				*parts == "8P" ){
					*parts = "MOVE_PUSH_DONE";
					move_push_done_set_flg = true;
			}
		}

		bool command_set_ok = false;
		//�R�}���h�@COMMAND_+�R�}���h���Ŏw�肷��		��FCOMMAND_236A
		//���x�����_���@COMMAND_+����+ATTACK_RAND		��FCOMMAND_236ATTACK_RAND
		if( strinc( *parts, "COMMAND_" ) ){
			if( strinc( *parts, "ATTACK_RAND_P" ) ){		//P�����_��
				string ratk;
				int strength = rand()%4;
				switch( strength ){
				case 0:
					ratk = "a";
					break;
				case 1:
					ratk = "b";
					break;
				case 2:
					ratk = "c";
					break;
				case 3:
					ratk = "ab";
					break;
				}
				
				string cmd( *parts, sizeof("COMMAND_")-1, parts->size() - sizeof("COMMAND_")+1 - sizeof("ATTACK_RAND_P")+1 );
				cmdname = ( cmd + ratk );		//COMMAND_236�Ƃ��������Ă���͂��Ȃ̂ŋ��x�𑫂�
				me->command.strength_check = (COMMAND_STRENGTH_CHECK)strength;	//���x�𔻕�
				me->command.commandname[0] = &cmdname;
				me->command.find_command_count = 1;
				command_set_ok = true;
			}else if( strinc( *parts, "ATTACK_RAND_K" ) ){		//K�����_��
				string ratk;
				int strength = rand()%4;
				switch( strength ){
				case 0:
					ratk = "d";
					break;
				case 1:
					ratk = "e";
					break;
				case 2:
					ratk = "f";
					break;
				case 3:
					ratk = "de";
					break;
				}
				
				string cmd( *parts, sizeof("COMMAND_")-1, parts->size() - sizeof("COMMAND_")+1 - sizeof("ATTACK_RAND_K")+1 );
				cmdname = ( cmd + ratk );		//COMMAND_236�Ƃ��������Ă���͂��Ȃ̂ŋ��x�𑫂�
				me->command.strength_check = (COMMAND_STRENGTH_CHECK)strength;	//���x�𔻕�
				me->command.commandname[0] = &cmdname;
				me->command.find_command_count = 1;
				command_set_ok = true;
			}else{
				//���ʂɐݒ�
				string cmd( *parts, sizeof("COMMAND_")-1, parts->size() - sizeof("COMMAND_")+1 );
				cmdname = cmd;
				me->command.commandname[0] = &cmdname;
				me->command.find_command_count = 1;

				//���x�𔻕�
				char s_strength = cmdname[ cmdname.size() - 1 ];
				COMMAND_STRENGTH_CHECK i_strength = COMMAND_STRENGTH_CHECK_L;
				if( strcmp( &s_strength, "a" ) == 0 || strcmp( &s_strength, "d" ) == 0 ){
					i_strength = COMMAND_STRENGTH_CHECK_L;
				}else if( strcmp( &s_strength, "b" ) == 0 || strcmp( &s_strength, "e" ) == 0 ){
					i_strength = COMMAND_STRENGTH_CHECK_M;
				}else if( strcmp( &s_strength, "c" ) == 0 || strcmp( &s_strength, "f" ) == 0 ){
					i_strength = COMMAND_STRENGTH_CHECK_H;
				}else{
					i_strength = COMMAND_STRENGTH_CHECK_EX;
				}
				me->command.strength_check = i_strength;
				command_set_ok = true;

				for( int i=0 ; i<me->command.skillcommand.size() ; i++ ){	//�h���ŕK�v������
					if( *me->command.commandname[0] == me->command.commandproperty[i].motion_name ){
						me->command.commandpriority[0] = i;
						break;
					}
				}
			}
		}

		//�U���{�^���ݒ��ATTACK_DONE�ɂȂ�
		if( me->button->push.flag[BUTTON_ATTACK1] ||
			me->button->push.flag[BUTTON_ATTACK2] ||
			me->button->push.flag[BUTTON_ATTACK3] ||
			me->button->push.flag[BUTTON_ATTACK4] ||
			me->button->push.flag[BUTTON_ATTACK5] ||
			me->button->push.flag[BUTTON_ATTACK6] ||
			command_set_ok ){
			*parts = "ATTACK_DONE";
			attack_done_set_flg = true;
			//advanceAi++;
		}
	}

	if( !wait_flg && !repeat_flg ){
		//���ʂɐi�߂�
		if( !me->down_flg ){
			
			bool ai_end_check = false;

			//�U�����@��{�I�ɂ́���_END�����Ȃ�����ǂݐi�߂�
			//if(me->attack_flg){	//�U���̎�
			if( !attack_done_set_flg && !move_push_done_set_flg ){		//�������t���[���͓���Ȃ��悤��
				if( *parts == "ATTACK_DONE" ){
					if( me->skinmesh.meshtimer < 4 && !( me->skinmesh.meshtimer == 2 && me->skinmesh.motion_name == "stand_wait" ) ){
						if( me->attack_flg ){
							attack_start_name = me->skinmesh.motion_name;
						}
					}else{
						if( attack_start_name != me->skinmesh.motion_name ){	//��~���Ȃ��悤�ɋ����I��
							advanceAi = 99;
						}
					}

					//�U���q�b�g��
					if( me->attack_hit ){
						if( 1/*attack_hit_past_1F*/ ){		//1F�҂�
							int advanceAi_bak = advanceAi;
							//�K�[�h���q�b�g
							if( enemy->guard_flg ){	//�K�[�h���ꂽ�Ƃ��p�b�N�I��
								if( function != NULL && *function == "PACK_END_GUARD" ){
									advanceAi = 99;
								}
								if( function != NULL && (*function == "CANCEL" || *function == "CANCEL_GUARD_ONLY") ){
									advanceAi += 2;
								}
							}else{		//�q�b�g�����Ƃ��p�b�N�I��
								if( function != NULL && *function == "PACK_END_HIT" ){
									advanceAi = 99;
								}
								if( function != NULL && (*function == "CANCEL" || *function == "CANCEL_HIT_ONLY") ){
									advanceAi += 2;
								}
							}
							if( function != NULL && (*function == "PACK_END_SUKA" || *function == "PACK_END_HIT" || *function == "PACK_END_GUARD") ){
								advanceAi += 2;

								if( argument ){
									if( *argument == "CANCEL" ){
										advanceAi++;
									}else if( *argument == "CANCEL_HIT_ONLY" ){
										if( enemy->damage_flg ){
											advanceAi++;
										}
									}else if( *argument == "CANCEL_GUARD_ONLY" ){
										if( enemy->guard_flg ){
											advanceAi++;
										}
									}
								}
							}

							ai_end_check = true;
							if( advanceAi_bak != advanceAi ){		//advanceAi���ύX���ꂽ��߂�
								attack_hit_past_1F = false;		//������g����悤��
							}
						}else{
							attack_hit_past_1F = true;
						}
					}

					//���n�҂�

					//���n�I��
					if( function != NULL && (*function == "PACK_END_LAND") ){
						if( me->vecPos.y >= ground ){
							if( me->attack_flg ){
								if( me->land_count == 1 ){	//���n�d�����l������
									advanceAi = 99;
									ai_end_check = true;
								}
							}else{
								if( me->land_count == 3 ){
									advanceAi = 99;
									ai_end_check = true;
								}
							}
						}
					}

					//���[�V�������I���ɒB����
					if( motion_end ){
						if( function != NULL && (*function == "CANCEL" || *function == "CANCEL_HIT_ONLY" || *function == "CANCEL_GUARD_ONLY") ){
							if( !me->attack_hit ){	//�L�����Z���\��̂Ƃ��p�b�N�I��
								advanceAi = 99;
							}else{
								advanceAi++;
							}
						}else if( function != NULL && (*function == "PACK_END_SUKA" || *function == "PACK_END_HIT" || *function == "PACK_END_GUARD") ){
							if( !me->attack_hit ){	//��U��̂Ƃ��p�b�N�I��
								advanceAi = 99;
							}else{
								advanceAi++;
							}
						}else{
							advanceAi++;
						}
						ai_end_check = true;
					}
				}else if( *parts == "MOVE_PUSH_DONE" ){
					advanceAi++;
				}else if( *parts == "ATTACK_END_WAIT" ){
					if( motion_end ){		//���[�V�������I���ɒB����
						advanceAi++;				//�U���I����҂��Ă���i�߂�
					}
				}else if( *parts == "LAND_WAIT" ){	//���n�҂� 10/26
					if( me->air_flg ){
						if( me->attack_flg ){
							if( me->land_count == 0 ){	//���n�d�����l������
								advanceAi++;
							}
						}else{
							if( me->land_count == 3 ){
								advanceAi++;
							}
						}
					}else{
						advanceAi++;		//�󒆂ł͂Ȃ��@���ɒ��n���Ă���H�̂Ői�߂�
					}
				}else{
					/*if(*parts == "5" || strinc( *parts, "8" )){
					gonext = true;
					}*/


					//strinc�͌����Ȃ̂ŁA�ق��̂o�������߂Ɣ��
					//���o�[�������ꂽ�u�Ԃ́i�����j�o�Ŕ��ʂ����ق�����������
					if( strinc( *parts,"P" ) ){		//���o�[�������ꂽ�u��
					}

					/*if( *parts == "2P" ||
					*parts == "4P" ||
					*parts == "6P" ||
					*parts == "8P" ){
					advanceAi++;
					}*/

					if( !me->attack_hit && motion_end ){	//�q�b�g�������[�V�������I���ɒB����
						advanceAi++;
					}

					ai_end_check = true;

				}
			}



			//AI�I���`�F�b�N�E���X�^�[�g
			if( ai_end_check ){
				if( advanceAi >= use_pack.parts.size() ){
					AiStart();
					ai_change_1F = true;
				}
			}

		}
	}




	//�L�[�{�[�h�������ꂽ���Ƃɂ���
	if( me->direction == DIRECTION::DIRECTION_LEFT ){
		me->button->press.flag[LEVER_RIGHT]		= me->frontpress;
		me->button->press.flag[LEVER_LEFT]		= me->backpress;
		me->button->push.flag[LEVER_RIGHT]		= me->frontpush;
		me->button->push.flag[LEVER_LEFT]		= me->backpush;
	}else{
		me->button->press.flag[LEVER_LEFT]		= me->frontpress;
		me->button->press.flag[LEVER_RIGHT]		= me->backpress;
		me->button->push.flag[LEVER_LEFT]		= me->frontpush;
		me->button->push.flag[LEVER_RIGHT]		= me->backpush;
	}
	//�G������ł閔�͎���������ł鎞�͑ҋ@
	if( enemy->death_flg || me->death_flg ){
		me->button->press.flag[LEVER_LEFT]		= false;
		me->button->press.flag[LEVER_RIGHT]		= false;
		me->button->press.flag[LEVER_UP]		= false;
		me->button->press.flag[LEVER_DOWN]		= false;
		me->button->press.flag[BUTTON_ATTACK1]	= false;
		me->button->press.flag[BUTTON_ATTACK2]	= false;
		me->button->press.flag[BUTTON_ATTACK3]	= false;
		me->button->press.flag[BUTTON_ATTACK4]	= false;
		me->button->press.flag[BUTTON_ATTACK5]	= false;
		me->button->press.flag[BUTTON_ATTACK6]	= false;

		me->frontpush	= false;
		me->backpush	= false;

		me->button->push.flag[BUTTON_ATTACK1]	= false;
		me->button->push.flag[BUTTON_ATTACK2]	= false;
		me->button->push.flag[BUTTON_ATTACK3]	= false;
		me->button->push.flag[BUTTON_ATTACK4]	= false;
		me->button->push.flag[BUTTON_ATTACK5]	= false;
		me->button->push.flag[BUTTON_ATTACK6]	= false;
		//command.commandname = NO_COMMAND;
	}
}


//�K�[�h
void CPU_AI::AiGuard(){
	//�U���҂̍U���f�[�^���擾
	AttackActionData* pAttackaction = NULL;
	if( enemy->attackaction_index != INVALID_VAL ){
		pAttackaction = &enemy->attackaction.attackactiondata[enemy->attackaction_index];
	}
	//int step = enemy->charadata.attackmotiondata[enemy->m_Chara->GetAnim()->m_CurAnimID].step;

	//�G���U�������H
	bool enemyattack = false;
	if( !enemy->damage_flg && enemy->attack_hitbox_appear ){
		enemyattack = true;
	}
	if( enemy->attack_flg ){
		enemyattack = true;
	}

	//�K�[�h���邩
	bool guard_try = false;
	if( enemyattack ){
		int dif_rand = rand()%100;
		if( dif_rand < 33 ){
			guard_try = true;
		}
	}

	//�K�[�h���s
	if( guard_try && !me->air_flg && !me->attack_flg ){
		//�K�[�h�i�̐؂�ւ�
		HITLEVEL hitlevel = HITLEVEL_MID;	//�f�t�H���g
		if( pAttackaction ){
			hitlevel = pAttackaction->hitlevel;	//�U���҂̍U���i
		}
		if( hitlevel == HITLEVEL_LOW ){
			me->button->press.flag[LEVER_DOWN] = true;		//���i�����̂ł��Ⴊ��
		}
		if( hitlevel == HITLEVEL_OVERHEAD ){
			me->button->press.flag[LEVER_DOWN] = false;		//���i�����̂ŗ���
		}

		//������
		me->backpress = true;
		if( me->direction == DIRECTION::DIRECTION_LEFT ){
			me->button->press.flag[LEVER_LEFT] = true;
		}else{
			me->button->press.flag[LEVER_RIGHT] = true;
		}
	}
}





//===================================================================
//�@�ȉ��_�~�[
//===================================================================
void CPU_AI::DummyProcess(){
	trainingdata = TrainingData::GetInst();

	//�L�[�{�[�h�߂�
	me->button->press.flag[LEVER_LEFT] = false;
	me->button->press.flag[LEVER_RIGHT] = false;
	me->button->press.flag[LEVER_UP] = false;
	me->button->press.flag[LEVER_DOWN] = false;
	me->button->press.flag[BUTTON_ATTACK1] = false;
	me->button->press.flag[BUTTON_ATTACK2] = false;
	me->button->press.flag[BUTTON_ATTACK3] = false;
	me->button->press.flag[BUTTON_ATTACK4] = false;
	me->button->press.flag[BUTTON_ATTACK5] = false;
	me->button->press.flag[BUTTON_ATTACK6] = false;

	me->frontpush = false;
	me->backpush = false;

	me->button->push.flag[BUTTON_ATTACK1] = false;
	me->button->push.flag[BUTTON_ATTACK2] = false;
	me->button->push.flag[BUTTON_ATTACK3] = false;
	me->button->push.flag[BUTTON_ATTACK4] = false;
	me->button->push.flag[BUTTON_ATTACK5] = false;
	me->button->push.flag[BUTTON_ATTACK6] = false;
	
	//�O��̐ݒ�
	if( me->direction == DIRECTION::DIRECTION_LEFT ){
		me->frontpress = me->button->press.flag[LEVER_RIGHT];
		me->backpress = me->button->press.flag[LEVER_LEFT];
	}else{
		me->frontpress = me->button->press.flag[LEVER_LEFT];
		me->backpress = me->button->press.flag[LEVER_RIGHT];
	}

	//�X�^���X
	switch( trainingdata->dummy_stance ){
	case TRAINING_DUMMY_STANCE_STAND:
		break;
	case TRAINING_DUMMY_STANCE_CROUCH:
		me->button->press.flag[LEVER_DOWN]	= true;
		break;
	case TRAINING_DUMMY_STANCE_JUMP:
		me->button->press.flag[LEVER_UP]	= true;
		break;
	}

	//�K�[�h
	bool guard_try = false;
	switch( trainingdata->dummy_guard ){
	case TRAINING_DUMMY_GUARD_NO:
		break;
	case TRAINING_DUMMY_GUARD_1HIT:

		break;
	case TRAINING_DUMMY_GUARD_SUBETE:
		break;
	case TRAINING_DUMMY_GUARD_RANDOM:
		break;
	}

	if( guard_try ){
		me->backpress = true;
	}
		
	//�L�[�{�[�h�������ꂽ���Ƃɂ���
	if (me->direction == DIRECTION::DIRECTION_LEFT ){
		me->button->press.flag[LEVER_RIGHT] = me->frontpress;
		me->button->press.flag[LEVER_LEFT] = me->backpress;
		me->button->push.flag[LEVER_RIGHT] = me->frontpush;
		me->button->push.flag[LEVER_LEFT] = me->backpush;
	}else{
		me->button->press.flag[LEVER_LEFT] = me->frontpress;
		me->button->press.flag[LEVER_RIGHT] = me->backpress;
		me->button->push.flag[LEVER_LEFT] = me->frontpush;
		me->button->push.flag[LEVER_RIGHT] = me->backpush;
	}

	DummyGuardCancel();
	DummyDamage();

	if( enemy->superstop_time <= 0 && me->superstop_time <= 0 && me->hitstop_time <= 0 ){
		trainingdata->dummy_1hit_guard--;
	}
}


//�_�~�[�K�[�h�|�[�Y
void CPU_AI::DummyGuardPause(){
	bool doBackPress = false;
	if( enemy->guardPauseTime > 0 ){
		if( fabs( me->vecPos.x - enemy->vecPos.x ) < enemy->guardPauseDistance ){
			doBackPress = true;
		}
	}


	//�K�[�h�|�[�Y���s
	if( doBackPress ){
		me->backpress = true;
		if( me->direction == DIRECTION::DIRECTION_LEFT ){
			me->button->press.flag[LEVER_LEFT] = true;
		}else{
			me->button->press.flag[LEVER_RIGHT] = true;
		}
	}
}
//�_�~�[�K�[�h
void CPU_AI::DummyGuard(){
	trainingdata = TrainingData::GetInst();

	//�U���҂̍U���f�[�^���擾
	AttackActionData* pAttackaction = NULL;
	if( enemy->attackaction_index != INVALID_VAL ){
		pAttackaction = &enemy->attackaction.attackactiondata[enemy->attackaction_index];
	}
	//int step = enemy->charadata.attackmotiondata[enemy->m_Chara->GetAnim()->m_CurAnimID].step;


	//�K�[�h���邩
	bool guard_try = false;
	int dif_rand = rand()%100;
	switch( trainingdata->dummy_guard ){
		case TRAINING_DUMMY_GUARD_NO:
			break;
		case TRAINING_DUMMY_GUARD_1HIT:
			if( trainingdata->dummy_1hit_guard > 0 ){
				guard_try = true;
			}
			break;
		case TRAINING_DUMMY_GUARD_SUBETE:
			guard_try = true;
			break;
		case TRAINING_DUMMY_GUARD_RANDOM:
			if( dif_rand < 50 ){
				guard_try = true;
			}
			break;
	}

	//n�i�ڃK�[�h�����s���悤�Ƃ���
	if( trainingdata->dummy_guard_failed > 0 ){
		if( me->ai.guard_success_count == trainingdata->dummy_guard_failed ){
			guard_try = false;	//�K�[�h���Ȃ�

			HITLEVEL hitlevel = HITLEVEL_MID;	//�f�t�H���g
			if( pAttackaction ){
				hitlevel = pAttackaction->hitlevel;	//�U���҂̍U���i
			}

			if( trainingdata->dummy_guard_step_change ){
				if( hitlevel == HITLEVEL_LOW ){
					me->button->press.flag[LEVER_DOWN] = false;
				}else if( hitlevel == HITLEVEL_OVERHEAD ){
					me->button->press.flag[LEVER_DOWN] = true;
				}
			}
		}
	}

	//�K�[�h�i�̐؂�ւ�
	if( guard_try ){
		HITLEVEL hitlevel = HITLEVEL_MID;	//�f�t�H���g
		if( pAttackaction ){
			hitlevel = pAttackaction->hitlevel;	//�U���҂̍U���i
		}

		if( trainingdata->dummy_guard_step_change ){
			switch( trainingdata->dummy_stance ){
			case TRAINING_DUMMY_STANCE_STAND:
			case TRAINING_DUMMY_STANCE_JUMP:
				if( hitlevel == HITLEVEL_LOW ){
					me->button->press.flag[LEVER_DOWN] = true;		//���i�����̂ł��Ⴊ��
				}
				break;
			case TRAINING_DUMMY_STANCE_CROUCH:
				if( hitlevel == HITLEVEL_OVERHEAD ){
					me->button->press.flag[LEVER_DOWN] = false;		//���i�����̂ŗ���
				}
				break;
			}
		}
	}

	//�G���U�������H
	bool enemyattack = false;
	if( !enemy->damage_flg && enemy->attack_hitbox_appear ){
		enemyattack = true;
	}
	if( enemy->attack_flg ){
		enemyattack = true;
	}
	
	//�K�[�h���s
	if( guard_try && !me->air_flg && !me->attack_flg /*&& enemyattack*/ ){
		me->backpress = true;
		if( me->direction == DIRECTION::DIRECTION_LEFT ){
			me->button->press.flag[LEVER_LEFT] = true;
		}else{
			me->button->press.flag[LEVER_RIGHT] = true;
		}
	}
}


void CPU_AI::DummyGuardCancel( bool CPU ){
	trainingdata = TrainingData::GetInst();

	//�K�[�h��
	if( me->guard_flg && me->knockback_time > 0 ){
		trainingdata->dummy_guard_progress_frame++;

		//GC
		bool guardcancel_try = false;
		int gc_rand = rand()%100;
		switch( trainingdata->dummy_guardcancel ){
			case TRAINING_DUMMY_GUARDCANCEL_NO:
				break;
			case TRAINING_DUMMY_GUARDCANCEL_SUBETE:
				if( trainingdata->dummy_guard_progress_frame == trainingdata->dummy_guardcancel_delay+1 ){
					guardcancel_try = true;
				}
				break;
			case TRAINING_DUMMY_GUARDCANCEL_DO_RANDOM:
				if( gc_rand < 25 ){
					if( trainingdata->dummy_guard_progress_frame == trainingdata->dummy_guardcancel_delay+1 ){
						guardcancel_try = true;
					}
				}
				break;
			case TRAINING_DUMMY_GUARDCANCEL_PERFECT_RANDOM:
				if( gc_rand < 25 ){
					guardcancel_try = true;
				}
				break;
		}
		//AI�p�����_�����s
		if( CPU ){
			guardcancel_try = false;
			int gc_rand = rand()%200;
			if( gc_rand < 3 ){	//�p���[�A�b�v�ɉ񂵂Ăق����̂ŁA����܂肵�Ă��Ȃ��悤��
				guardcancel_try = true;
			}
		}
		//GC���s
		if( guardcancel_try ){
			me->frontpress = true;
			me->frontpush = true;
			me->backpress = false;
			me->backpush = true;
			if( me->direction == DIRECTION::DIRECTION_LEFT ){
				me->button->press.flag[LEVER_RIGHT] = true;
				me->button->press.flag[LEVER_LEFT] = false;
			}else{
				me->button->press.flag[LEVER_LEFT] = true;
				me->button->press.flag[LEVER_RIGHT] = false;
			}
			//�Ƃ肠�����S������PPP�ł�KKK�ł��o��悤��
			me->button->push.flag[BUTTON_ATTACK1] = true;
			me->button->push.flag[BUTTON_ATTACK2] = true;
			me->button->push.flag[BUTTON_ATTACK3] = true;
			me->button->push.flag[BUTTON_ATTACK4] = true;
			me->button->push.flag[BUTTON_ATTACK5] = true;
			me->button->push.flag[BUTTON_ATTACK6] = true;
		}
	}else{
		trainingdata->dummy_guard_progress_frame = 0;
	}
}


void CPU_AI::DummyDamage( bool CPU ){
	if( me->damage_flg ){
		bool ukemi_try = false;
		int gc_rand = rand()%100;
		switch( trainingdata->dummy_recovery ){
			case TRAINING_DUMMY_RECOVERY_NO:
				break;
			case TRAINING_DUMMY_RECOVERY_SUBETE:
				if( trainingdata->dummy_recovery_delay == (ukemi_yuuyo - me->ukemi_time) ){
					ukemi_try = true;
				}
				break;
			case TRAINING_DUMMY_RECOVERY_DO_RANDOM:
				if( gc_rand < 50 ){
					if( trainingdata->dummy_recovery_delay == (ukemi_yuuyo - me->ukemi_time) ){
						ukemi_try = true;
					}
				}
				break;
			case TRAINING_DUMMY_RECOVERY_PERFECT_RANDOM:
				if( gc_rand < 50 ){
					ukemi_try = true;
				}
				break;
		}
		//AI�p�����_�����s
		if( CPU ){
			ukemi_try = false;
			int ukm_rand = rand()%7;
			if( ukm_rand < 5 ){	//�p���[�A�b�v�ɉ񂵂Ăق����̂ŁA����܂肵�Ă��Ȃ��悤��
				ukemi_try = true;
			}
		}
		if( ukemi_try ){
			me->button->push.flag[LEVER_DOWN] = true;
		}
	}
}




void CPU_AI::OH2_AI(){
	me->button->press.flag[LEVER_LEFT] = false;
	me->button->press.flag[LEVER_RIGHT] = false;
	me->button->press.flag[LEVER_UP] = false;
	me->button->press.flag[LEVER_DOWN] = false;
	me->button->press.flag[BUTTON_ATTACK1] = false;
	me->button->press.flag[BUTTON_ATTACK2] = false;
	me->button->press.flag[BUTTON_ATTACK3] = false;
	me->button->press.flag[BUTTON_ATTACK4] = false;
	me->button->press.flag[BUTTON_ATTACK5] = false;
	me->button->press.flag[BUTTON_ATTACK6] = false;

	me->frontpush = false;
	me->backpush = false;

	me->button->push.flag[LEVER_LEFT] = false;
	me->button->push.flag[LEVER_RIGHT] = false;
	me->button->push.flag[LEVER_UP] = false;
	me->button->push.flag[LEVER_DOWN] = false;
	me->button->push.flag[BUTTON_ATTACK1] = false;
	me->button->push.flag[BUTTON_ATTACK2] = false;
	me->button->push.flag[BUTTON_ATTACK3] = false;
	me->button->push.flag[BUTTON_ATTACK4] = false;
	me->button->push.flag[BUTTON_ATTACK5] = false;
	me->button->push.flag[BUTTON_ATTACK6] = false;


	int rand_LR = rand()%417;
	if( rand_LR < 5 ){
		me->button->press.flag[LEVER_LEFT] = true;
		me->button->push.flag[LEVER_LEFT] = true;
		ai_oh2[0] = rand()%60 + 8;
		ai_oh2[1] = 0;
	}else if( rand_LR < 10 ){
		me->button->press.flag[LEVER_RIGHT] = true;
		me->button->push.flag[LEVER_RIGHT] = true;
		ai_oh2[0] = 0;
		ai_oh2[1] = rand()%60 + 8;
	}

	int rand_U = rand()%413;
	if( rand_U < 5 ){
		me->button->press.flag[LEVER_UP] = true;
		me->button->push.flag[LEVER_UP] = true;
	}

	int rand_D = rand()%405;
	if( rand_D < 7 ){
		me->button->press.flag[LEVER_DOWN] = true;
		me->button->push.flag[LEVER_DOWN] = true;
		ai_oh2[2] = rand()%64 + 14;
	}

	//�������ψ���
	if( ai_oh2[0] > 0 ){
		me->button->press.flag[LEVER_LEFT] = true;
	}else if( ai_oh2[1] > 0 ){
		me->button->press.flag[LEVER_RIGHT] = true;
	}
	if( ai_oh2[2] > 0 ){
		me->button->press.flag[LEVER_DOWN] = true;
	}
	ai_oh2[0]--;
	ai_oh2[1]--;
	ai_oh2[2]--;


	//�O��̐ݒ�
	if( me->direction == DIRECTION::DIRECTION_LEFT ){
		me->frontpress = me->button->press.flag[LEVER_RIGHT];
		me->backpress = me->button->press.flag[LEVER_LEFT];
	}else{
		me->frontpress = me->button->press.flag[LEVER_LEFT];
		me->backpress = me->button->press.flag[LEVER_RIGHT];
	}

	//ATK
	//�K���ɓ����������N����悤��
	int rand_ATK = rand()%377;
	if( rand_ATK < 9 ){
		for( int i=0 ; i<6 ; i++ ){
			rand_ATK = rand()%17;
			if( rand_ATK < 5 ){
				me->button->press.flag[BUTTON_ATTACK1 + i] = true;
				me->button->push.flag[BUTTON_ATTACK1 + i] = true;
			}
		}
	}
}




//�h���R�}���h�p
void CPU_AI::ADD_COMMAND_AI(){
	int dif_rand = ((4-GameOption::GetInst()->optionitem.difficulty)+1)*5;
	int dif_rand2 = rand() % dif_rand;

	switch( me->charanum ){
		case 0:
			if( dif_rand2 < 10 ){
				if(	me->skinmesh.motion_name == "down_faceup" ){

				}
			}
			break;
	}
}


//�e�F��
void CPU_AI::ShotRecognition(){
	//�e����������ꍇ�͍ŏ��ɔ��˂��ꂽ�e���������邱�ƂɂȂ�

	bool shotRangeYellow	= false;	//����
	bool shotRangeRed		= false;	//�댯

	if( 
		recognitionShotIndex < 0 || recognitionShotIndex >= SHOT_USE_MAX ||
		!enemy->shot[recognitionShotIndex] ||
		enemy->shot[recognitionShotIndex]->sparam.delete_flg
		){
		recognitionShotIndex = INVALID_VAL;
		doneRecognitionShotGuard = false;
	}

	if( recognitionShotIndex == INVALID_VAL ){
		for( int i=0 ; i<SHOT_USE_MAX ; i++ ){
			if( enemy->shot[i] ){
				if( enemy->shot[i]->sparam.enable && !enemy->shot[i]->sparam.delete_flg ){
					recognitionShotIndex = i;
					doneRecognitionShotGuard = false;
					break;
				}
			}
		}
	}

	if( recognitionShotIndex > 0 ){
		ShotBase* pShot = enemy->shot[recognitionShotIndex];

		//�i�e�̈ʒu�{�e�̉����x�j�Ǝ����Ƃ̋���
		float shot_pos_x = pShot->sparam.pos.x - pShot->sparam.ofs.x /*- pShot->sparam.dx*/ + enemy->pView_camera->x;
		float shot_pos_y = pShot->sparam.pos.y - pShot->sparam.ofs.y /*- pShot->sparam.dy*/ + enemy->pView_camera->y;

		int shotX = abs( shot_pos_x - me->vecPos.x );
		if( shotX < 320 ){
			shotRangeRed = true;
		}else if( shotX < 770 ){
			shotRangeYellow = true;
		}

		//���������Ċm�����o��
		int doGuardPercent	= INVALID_VAL;
		int doJumpPercent	= INVALID_VAL;
		if( shotRangeRed ){
			doGuardPercent = 60;
			doJumpPercent = 30;
		}else if( shotRangeYellow ){
			doGuardPercent = 20;
			doJumpPercent = 60;
		}
		

		bool doJump = false;
		if( !doneRecognitionShotGuard ){
			int recognitionShotGuard = rand()%100;
			if( recognitionShotGuard < doGuardPercent ){
				doneRecognitionShotGuard = true;
			}else{
				//�͂��ꂽ�炳��ɒ��I�i������true�ɂȂ�Ȃ��悤�Ɂj
				int recognitionShotJump = rand()%100;
				if( recognitionShotJump < doJumpPercent ){
					doJump = true;
				}
			}
		}


		if( doneRecognitionShotGuard ){
			//me->button->press.flag[LEVER_DOWN] = true;
			//������
			me->backpress = true;
			if( me->direction == DIRECTION::DIRECTION_LEFT ){
				me->button->press.flag[LEVER_LEFT] = true;
			}else{
				me->button->press.flag[LEVER_RIGHT] = true;
			}
		}else if( doJump ){
			me->button->press.flag[LEVER_UP] = true;
			//�i�s����
			if( rand()%5 < 2 ){
				me->frontpress = true;
				if( me->direction == DIRECTION::DIRECTION_LEFT ){
					me->button->press.flag[LEVER_RIGHT] = true;
				}else{
					me->button->press.flag[LEVER_LEFT] = true;
				}
			}
		}
	}
}

//�v���C���[����
void CPU_AI::PlayerAnalysis(){
	//�U���{�^���������ꂽ��
	for( int i=0 ; i<6 ; i++ ){
		if( enemy->button->push.flag[(int)BUTTON_ATTACK1 + i] ){
			aiAnalysis[CPU_PLAYER_ANALYSIS_PRESS_ATTACK_KEY]++;
			break;		//1F�Ɉ�񂾂�
		}
	}
}



//�{�X�p�@���ł�
void CPU_AI::BOSS_AI(){
	me->button->press.flag[LEVER_LEFT] = false;
	me->button->press.flag[LEVER_RIGHT] = false;
	me->button->press.flag[LEVER_UP] = false;
	me->button->press.flag[LEVER_DOWN] = false;
	me->button->press.flag[BUTTON_ATTACK1] = false;
	me->button->press.flag[BUTTON_ATTACK2] = false;
	me->button->press.flag[BUTTON_ATTACK3] = false;
	me->button->press.flag[BUTTON_ATTACK4] = false;
	me->button->press.flag[BUTTON_ATTACK5] = false;
	me->button->press.flag[BUTTON_ATTACK6] = false;

	me->frontpush = false;
	me->backpush = false;

	me->button->push.flag[LEVER_LEFT] = false;
	me->button->push.flag[LEVER_RIGHT] = false;
	me->button->push.flag[LEVER_UP] = false;
	me->button->push.flag[LEVER_DOWN] = false;
	me->button->push.flag[BUTTON_ATTACK1] = false;
	me->button->push.flag[BUTTON_ATTACK2] = false;
	me->button->push.flag[BUTTON_ATTACK3] = false;
	me->button->push.flag[BUTTON_ATTACK4] = false;
	me->button->push.flag[BUTTON_ATTACK5] = false;
	me->button->push.flag[BUTTON_ATTACK6] = false;

	//�ړ�
	int rand_LR = rand()%417;
	if( rand_LR < 5 ){
		me->button->press.flag[LEVER_LEFT] = true;
		me->button->push.flag[LEVER_LEFT] = true;
		ai_oh2[0] = rand()%60 + 8;
		ai_oh2[1] = 0;
	}else if( rand_LR < 10 ){
		me->button->press.flag[LEVER_RIGHT] = true;
		me->button->push.flag[LEVER_RIGHT] = true;
		ai_oh2[0] = 0;
		ai_oh2[1] = rand()%60 + 8;
	}

	//���
	int rand_U = rand()%413;
	if( rand_U < 5 ){
		me->button->press.flag[LEVER_UP] = true;
		me->button->push.flag[LEVER_UP] = true;
	}

	//���Ⴊ��
	int rand_D = rand()%405;
	if( rand_D < 7 ){
		me->button->press.flag[LEVER_DOWN] = true;
		me->button->push.flag[LEVER_DOWN] = true;
		ai_oh2[2] = rand()%64 + 14;
	}

	//�������ψ���
	if( ai_oh2[0] > 0 ){
		me->button->press.flag[LEVER_LEFT] = true;
	}else if( ai_oh2[1] > 0 ){
		me->button->press.flag[LEVER_RIGHT] = true;
	}
	if( ai_oh2[2] > 0 ){
		me->button->press.flag[LEVER_DOWN] = true;
	}
	ai_oh2[0]--;
	ai_oh2[1]--;
	ai_oh2[2]--;


	//�O��̐ݒ�
	if( me->direction == DIRECTION::DIRECTION_LEFT ){
		me->frontpress = me->button->press.flag[LEVER_RIGHT];
		me->backpress = me->button->press.flag[LEVER_LEFT];
	}else{
		me->frontpress = me->button->press.flag[LEVER_LEFT];
		me->backpress = me->button->press.flag[LEVER_RIGHT];
	}


	//����
	float kyori = abs( me->vecPos.x - enemy->vecPos.x );

	//ATK
	if( kyori < 200 ){
		me->button->press.flag[BUTTON_ATTACK1] = true;
		me->button->push.flag[BUTTON_ATTACK1] = true;
	}

	//��肱�ݏ���
	if( kyori < 200 ){
		if( enemy->attack_flg ){
			me->button->press.flag[BUTTON_ATTACK1] = true;
			me->button->push.flag[BUTTON_ATTACK1] = true;
		}
	}

}





