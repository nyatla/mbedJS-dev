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
#include "../NyLPC_cMutex.h"
#if NyLPC_ARCH==NyLPC_ARCH_FREERTOS

NyLPC_TBool NyLPC_cMutex_initialize(NyLPC_TcMutex_t* i_inst)
{
	i_inst->_mutex=xSemaphoreCreateRecursiveMutex();
	i_inst->_lock_count=0;
	return i_inst->_mutex!=NULL;
}
NyLPC_TBool NyLPC_cMutex_lock(NyLPC_TcMutex_t* i_inst)
{
	if(xSemaphoreTakeRecursive(i_inst->_mutex,portMAX_DELAY) == pdTRUE)
	{
		i_inst->_lock_count++;
		return NyLPC_TBool_TRUE;
	}
	return NyLPC_TBool_FALSE;
}
NyLPC_TBool NyLPC_cMutex_unlock(NyLPC_TcMutex_t* i_inst)
{
	NyLPC_Assert(i_inst->_lock_count>0);
	i_inst->_lock_count--;
	return xSemaphoreGiveRecursive( i_inst->_mutex ) == pdTRUE;
}
#endif
