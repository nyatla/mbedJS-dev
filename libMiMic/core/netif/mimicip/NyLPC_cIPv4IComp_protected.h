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
 *  http://nyatla.jp/
 *  <airmail(at)ebony.plala.or.jp> or <nyatla(at)nyatla.jp>
 *
 *
 * Parts of this file were leveraged from uIP:
 *
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef NYLPC_NyLPC_cIPv4IComp_protected_H_
#define NYLPC_NyLPC_cIPv4IComp_protected_H_

#include "NyLPC_cIPv4Payload_protected.h"
#include "../NyLPC_NetIf_ip_types.h"
#include "NyLPC_cIPv4IComp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

NyLPC_TBool NyLPC_cIPv4IComp_initialize(
    NyLPC_TcIPv4IComp_t* i_inst,
    const NyLPC_TcIPv4Config_t* i_ref_config);

void NyLPC_cIPv4IComp_finalize(
    NyLPC_TcIPv4IComp_t* i_inst);

/**
 * この関数は、ICOMP IPv4パケットを処理します。
 * @return
 * イーサネットパケットを格納したメモリを返します。
 * このメモリはNyLPC_cUipService_allocTxBufで取得されたメモリです。
 */
void* NyLPC_cIPv4IComp_rx(
    const NyLPC_TcIPv4IComp_t* i_inst,
    const NyLPC_TcIPv4Payload_t* i_ipp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

