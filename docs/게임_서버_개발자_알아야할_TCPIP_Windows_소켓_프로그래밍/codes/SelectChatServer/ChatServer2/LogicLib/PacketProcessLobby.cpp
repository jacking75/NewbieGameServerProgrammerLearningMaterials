#include <tuple>

#include "../../Common/Packet.h"
#include "../../Common/ErrorCode.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "User.h"
#include "UserManager.h"
#include "Lobby.h"
#include "LobbyManager.h"
#include "PacketProcess.h"

using PACKET_ID = NCommon::PACKET_ID;

namespace NLogicLib
{
	ERROR_CODE PacketProcess::LobbyEnter(PacketInfo packetInfo)
	{
        auto reqPkt = (NCommon::PktLobbyEnterReq*)packetInfo.pRefData;
		NCommon::PktLobbyEnterRes resPkt;

		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
		if (errorCode != ERROR_CODE::NONE) {
            resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
            return errorCode;
		}
				
		if (pUser->IsCurDomainInLogIn() == false) {
			resPkt.SetError(ERROR_CODE::LOBBY_ENTER_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
            return ERROR_CODE::LOBBY_ENTER_INVALID_DOMAIN;
		}

		auto pLobby = m_pRefLobbyMgr->GetLobby(reqPkt->LobbyId);
		if (pLobby == nullptr) {
			resPkt.SetError(ERROR_CODE::LOBBY_ENTER_INVALID_LOBBY_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
            return ERROR_CODE::LOBBY_ENTER_INVALID_LOBBY_INDEX;
		}

		auto enterRet = pLobby->EnterUser(pUser);
		if (enterRet != ERROR_CODE::NONE) {
			resPkt.SetError(enterRet);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
            return enterRet;
		}

		resPkt.MaxUserCount = pLobby->MaxUserCount();
		resPkt.MaxRoomCount = pLobby->MaxRoomCount();
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_ENTER_RES, sizeof(NCommon::PktLobbyEnterRes), (char*)&resPkt);
		return ERROR_CODE::NONE;
	}
		
	ERROR_CODE PacketProcess::LobbyLeave(PacketInfo packetInfo)
	{
		NCommon::PktLobbyLeaveRes resPkt;

		auto [errorCode, pUser] = m_pRefUserMgr->GetUser(packetInfo.SessionIndex);
		
        if (errorCode != ERROR_CODE::NONE) {
			resPkt.SetError(errorCode);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
            return errorCode;
		}

		if (pUser->IsCurDomainInLobby() == false) {
			resPkt.SetError(ERROR_CODE::LOBBY_LEAVE_INVALID_DOMAIN);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
            return ERROR_CODE::LOBBY_LEAVE_INVALID_DOMAIN;
		}

		auto pLobby = m_pRefLobbyMgr->GetLobby(pUser->GetLobbyIndex());
		if (pLobby == nullptr) {
			resPkt.SetError(ERROR_CODE::LOBBY_LEAVE_INVALID_LOBBY_INDEX);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
            return ERROR_CODE::LOBBY_LEAVE_INVALID_LOBBY_INDEX;
		}

		auto enterRet = pLobby->LeaveUser(pUser->GetIndex());
		if (enterRet != ERROR_CODE::NONE) {
			resPkt.SetError(enterRet);
            m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
            return enterRet;
		}
						
		m_pRefNetwork->SendData(packetInfo.SessionIndex, (short)PACKET_ID::LOBBY_LEAVE_RES, sizeof(NCommon::PktLobbyLeaveRes), (char*)&resPkt);
		return ERROR_CODE::NONE;
	}	
}