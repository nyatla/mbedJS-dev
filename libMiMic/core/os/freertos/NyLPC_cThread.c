/*********************************************************************************
 * PROJECT: MiMic
 * --------------------------------------------------------------------------------
 *
 * This file is part of MiMic
 * Copyright (C)2011 Ryo Iizuka
 *
 * MiMic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by　the Free Software Foundation, either version 3 of the　License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information please contact.
 *	http://nyatla.jp/
 *	<airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *********************************************************************************/
#include "../NyLPC_cThread.h"
#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS

unsigned portBASE_TYPE prio_tbl[]={
	0U,	//0:idle
	5U,	//1:service
};


static void _task(void *pvParameters);

void NyLPC_cThread_initialize(NyLPC_TcThread_t* i_inst,NyLPC_TInt32 i_stack,NyLPC_TInt32 i_prio)
{
	NyLPC_TUInt8_setBit(i_inst->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED);
	NyLPC_AbortIfNot(pdPASS==xTaskCreate(
		_task,
		(const signed char*)"th",
		(i_stack<1)?NyLPC_TcThread_DEFAULT_STACKSIZE:i_stack,
		( void * ) i_inst,
		prio_tbl[i_prio],
		&(i_inst->_taskid)));
}
void NyLPC_cThread_finalize(NyLPC_TcThread_t* i_inst)
{
	vTaskDelete(i_inst->_taskid);
}

void NyLPC_cThread_start(NyLPC_TcThread_t* i_inst,NyLPC_TcThread_ThreadFunc i_func,void* i_param)
{
	NyLPC_ArgAssert(i_inst!=NULL);
	NyLPC_ArgAssert(i_func!=NULL);
	NyLPC_ArgAssert(NyLPC_TUInt32_isBitOn(i_inst->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED));

	i_inst->_func=i_func;
	i_inst->_param=i_param;

	portENTER_CRITICAL(); //<critical session>
	i_inst->_sbit=0;
	vTaskResume(i_inst->_taskid);
	portEXIT_CRITICAL(); //</critical session>

	return;
}
void NyLPC_cThread_join(NyLPC_TcThread_t* i_inst)
{
	NyLPC_TUInt32_setBit(i_inst->_sbit,NyLPC_TcThread_BIT_IS_JOIN_REQ);
	while(!NyLPC_TUInt32_isBitOn(i_inst->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED))
	{
		vTaskDelay(2);//少し待つ。
	}
	//終了
	return;
}
void NyLPC_cThread_sleep(NyLPC_TUInt32 i_time_in_msec)
{
	//ミリ秒単位で待つ
	vTaskDelay(i_time_in_msec/portTICK_RATE_MS);
}
void NyLPC_cThread_yield()
{
	taskYIELD();
}
static void _task(void *pvParameters)
{
	NyLPC_TcThread_t* inst=(NyLPC_TcThread_t*)pvParameters;

	portENTER_CRITICAL(); //<critical session>
	//Terminated-bitがONであれば停止する。
	if(NyLPC_TUInt32_isBitOn(inst->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED)){
		vTaskSuspend(inst->_taskid);
	}
	portEXIT_CRITICAL(); //</critical session>

	for(;;){
		inst->_func(inst->_param);

		portENTER_CRITICAL();//<critical session>
		NyLPC_TUInt32_setBit(inst->_sbit,NyLPC_TcThread_BIT_IS_TERMINATED);
		vTaskSuspend(inst->_taskid);
		portEXIT_CRITICAL(); //</critical session>
	}
}
#endif
