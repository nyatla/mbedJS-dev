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
#include "../NyLPC_cSemaphore.h"
#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS

void NyLPC_cSemaphore_initialize(NyLPC_TcSemaphore_t* i_inst)
{
	i_inst->_handle=xSemaphoreCreateMutex();
	NyLPC_Assert(i_inst->_handle!=NULL);
}


void NyLPC_cSemaphore_giveFromISR(const NyLPC_TcSemaphore_t* i_inst)
{
	long lHigherPriorityTaskWoken = pdFALSE;
	//セマフォブロックの解除
	xSemaphoreGiveFromISR(i_inst->_handle, &lHigherPriorityTaskWoken );
}
/**
 * @param i_timeout
 * タイムアウト時間はms指定
 */
void NyLPC_cSemaphore_take(const NyLPC_TcSemaphore_t* i_inst,NyLPC_TUInt32 i_timeout)
{
	xSemaphoreTake(i_inst->_handle,i_timeout/portTICK_RATE_MS);
}
#endif
