using CSCommon;
using MessagePack;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace csharp_test_client
{
    public partial class mainForm
    {
        Dictionary<UInt16, Action<byte[]>> PacketFuncDic = new Dictionary<UInt16, Action<byte[]>>();

        void SetPacketHandler()
        {
            //PacketFuncDic.Add(PACKET_ID.PACKET_ID_ERROR_NTF, PacketProcess_ErrorNotify);
            PacketFuncDic.Add(PacketID.ResLogin, PacketProcess_Loginin);
            PacketFuncDic.Add(PacketID.ResLobbyEnter, PacketProcess_LobbyEnter);
            PacketFuncDic.Add(PacketID.NtfLobbyEnterNewUser, PacketProcess_LobbyEnterNewUser);
            PacketFuncDic.Add(PacketID.ResLobbyLeave, PacketProcess_LobbyLeave);
            PacketFuncDic.Add(PacketID.NtfLobbyLeaveUser, PacketProcess_LobbyLeaveUser);

            PacketFuncDic.Add(PacketID.NtfLobbyChat, PacketProcess_LobbyChat);
            //PacketFuncDic.Add(PacketID.ROOM_ENTER_RES, PacketProcess_RoomEnterResponse);
            //PacketFuncDic.Add(PacketID.ROOM_USER_LIST_NTF, PacketProcess_RoomUserListNotify);
            //PacketFuncDic.Add(PacketID.ROOM_NEW_USER_NTF, PacketProcess_RoomNewUserNotify);
            //PacketFuncDic.Add(PacketID.ROOM_LEAVE_RES, PacketProcess_RoomLeaveResponse);
            //PacketFuncDic.Add(PacketID.ROOM_LEAVE_USER_NTF, PacketProcess_RoomLeaveUserNotify);
            //PacketFuncDic.Add(PacketID.ROOM_CHAT_RES, PacketProcess_RoomChatResponse);            
            //PacketFuncDic.Add(PacketID.ROOM_CHAT_NOTIFY, PacketProcess_RoomChatNotify);
            //PacketFuncDic.Add(PacketID.MATCH_USER_RES, PacketProcess_MatchUserResponse);
            //PacketFuncDic.Add(PacketID.PUT_STONE_RES, PacketProcess_PutStoneResponse);
            //PacketFuncDic.Add(PacketID.GAME_END_RESULT, PacketProcess_GameEndResultResponse);
            //PacketFuncDic.Add(PacketID.GAME_START_RES, PacketProcess_GameStartResultResponse);
        }

        void PacketProcess(byte[] packet)
        {
            var header = new MsgPackPacketHeaderInfo();
            header.Read(packet);

            var packetID = header.ID;
            
            if (PacketFuncDic.ContainsKey(packetID))
            {
                PacketFuncDic[packetID](packet);
            }
            else
            {
                DevLog.Write("Unknown Packet Id: " + packetID);
            }
        }

        void PacketProcess_PutStoneInfoNotifyResponse(byte[] bodyData)
        {
            /*var responsePkt = new PutStoneNtfPacket();
            responsePkt.FromBytes(bodyData);

            DevLog.Write($"'{responsePkt.userID}' Put Stone  : [{responsePkt.xPos}] , [{responsePkt.yPos}] ");*/

        }

        void PacketProcess_GameStartResultResponse(byte[] bodyData)
        {
            /*var responsePkt = new GameStartResPacket();
            responsePkt.FromBytes(bodyData);

            if ((ERROR_CODE)responsePkt.Result == ERROR_CODE.NOT_READY_EXIST)
            {
                DevLog.Write($"모두 레디상태여야 시작합니다.");
            }
            else
            {
                DevLog.Write($"게임시작 !!!! '{responsePkt.UserID}' turn  ");
            }*/
        }

        void PacketProcess_GameEndResultResponse(byte[] bodyData)
        {
            /*var responsePkt = new GameResultResPacket();
            responsePkt.FromBytes(bodyData);
            
            DevLog.Write($"'{responsePkt.UserID}' WIN , END GAME ");*/

        }

        void PacketProcess_PutStoneResponse(byte[] bodyData)
        {
            /*var responsePkt = new MatchUserResPacket();
            responsePkt.FromBytes(bodyData);

            if((ERROR_CODE)responsePkt.Result != ERROR_CODE.ERROR_NONE)
            {
                DevLog.Write($"Put Stone Error : {(ERROR_CODE)responsePkt.Result}");
            }

            DevLog.Write($"다음 턴 :  {(ERROR_CODE)responsePkt.Result}");*/

        }

        void PacketProcess_MatchUserResponse(byte[] bodyData)
        {
            /*var responsePkt = new MatchUserResPacket();
            responsePkt.FromBytes(bodyData);

            DevLog.Write($"매칭 결과:  {(ERROR_CODE)responsePkt.Result} ");*/

        }


        void PacketProcess_ErrorNotify(byte[] packetData)
        {
            /*var notifyPkt = new ErrorNtfPacket();
            notifyPkt.FromBytes(bodyData);

            DevLog.Write($"에러 통보 받음:  {notifyPkt.Error}");*/
        }


        void PacketProcess_Loginin(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResLogin>(packetData);
            DevLog.Write($"로그인 결과: {(ErrorCode)responsePkt.Result}");
        }

        
        void PacketProcess_LobbyEnter(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResLobbyEnter>(packetData);
            DevLog.Write($"로비 들어가기 결과: {(ErrorCode)responsePkt.Result}");
        }

        void PacketProcess_LobbyEnterNewUser(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfLobbyEnterNewUser>(packetData);
            DevLog.Write($"로비에 새로 들어온 유저: {notifyPkt.UserID}");
        }

        void PacketProcess_LobbyLeave(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResLobbyLeave>(packetData);
            DevLog.Write($"로비 나가기 결과: {(ErrorCode)responsePkt.Result}");
        }

        void PacketProcess_LobbyLeaveUser(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfLobbyLeaveUser>(packetData);
            DevLog.Write($"로비에서 나간 유저: {notifyPkt.UserID}");
        }

        void PacketProcess_LobbyChat(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfLobbyChat>(packetData);
            AddLobbyChat(notifyPkt.UserID, notifyPkt.Message);
        }

        void PacketProcess_RoomEnterResponse(byte[] bodyData)
        {
            /*var responsePkt = new RoomEnterResPacket();
            responsePkt.FromBytes(bodyData);

            DevLog.Write($"방 입장 결과:  {(ERROR_CODE)responsePkt.Result}");*/
        }

        void PacketProcess_RoomUserListNotify(byte[] bodyData)
        {
            /*var notifyPkt = new RoomUserListNtfPacket();
            notifyPkt.FromBytes(bodyData);

            for (int i = 0; i < notifyPkt.UserCount; ++i)
            {
                AddRoomUserList(notifyPkt.UserUniqueIdList[i], notifyPkt.UserIDList[i]);
            }

            DevLog.Write($"방의 기존 유저 리스트 받음");*/
        }

        void PacketProcess_RoomNewUserNotify(byte[] bodyData)
        {
            /*var notifyPkt = new RoomNewUserNtfPacket();
            notifyPkt.FromBytes(bodyData);

            AddRoomUserList(notifyPkt.UserUniqueId, notifyPkt.UserID);
            
            DevLog.Write($"방에 새로 들어온 유저 받음");*/
        }


        void PacketProcess_RoomLeaveResponse(byte[] bodyData)
        {
            /*var responsePkt = new RoomLeaveResPacket();
            responsePkt.FromBytes(bodyData);

            DevLog.Write($"방 나가기 결과:  {(ERROR_CODE)responsePkt.Result}");*/
        }

        void PacketProcess_RoomLeaveUserNotify(byte[] bodyData)
        {
            /*var notifyPkt = new RoomLeaveUserNtfPacket();
            notifyPkt.FromBytes(bodyData);

            RemoveRoomUserList(notifyPkt.UserUniqueId);

            DevLog.Write($"방에서 나간 유저 받음");*/
        }


        void PacketProcess_RoomChatResponse(byte[] bodyData)
        {
            /*var responsePkt = new RoomChatResPacket();
            responsePkt.FromBytes(bodyData);

            var errorCode = (ERROR_CODE)responsePkt.Result;
            var msg = $"방 채팅 요청 결과:  {(ERROR_CODE)responsePkt.Result}";
            if (errorCode == ERROR_CODE.ERROR_NONE)
            {
                DevLog.Write(msg, LOG_LEVEL.ERROR);
            }
            else
            {
                AddRoomChatMessageList(0, msg);
            }*/
        }


        void PacketProcess_RoomChatNotify(byte[] bodyData)
        {
            /*var responsePkt = new RoomChatNtfPacket();
            responsePkt.FromBytes(bodyData);

            AddRoomChatMessageList(responsePkt.UserUniqueId, responsePkt.Message);*/
        }

        void AddRoomChatMessageList(Int64 userUniqueId, string msgssage)
        {
            /*var msg = $"{userUniqueId}:  {msgssage}";

            if (listBoxRoomChatMsg.Items.Count > 512)
            {
                listBoxRoomChatMsg.Items.Clear();
            }

            listBoxRoomChatMsg.Items.Add(msg);
            listBoxRoomChatMsg.SelectedIndex = listBoxRoomChatMsg.Items.Count - 1;*/
        }


        void PacketProcess_RoomRelayNotify(byte[] bodyData)
        {
            /*var notifyPkt = new RoomRelayNtfPacket();
            notifyPkt.FromBytes(bodyData);

            var stringData = Encoding.UTF8.GetString(notifyPkt.RelayData);
            DevLog.Write($"방에서 릴레이 받음. {notifyPkt.UserUniqueId} - {stringData}");*/
        }
    }
}
