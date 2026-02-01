using CSCommon;
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

            PacketFuncDic.Add(PacketID.ResRoomEnter, PacketProcess_RoomEnterResponse);
            PacketFuncDic.Add(PacketID.NtfRoomUserList, PacketProcess_RoomUserListNotify);
            PacketFuncDic.Add(PacketID.NtfRoomNewUser, PacketProcess_RoomNewUserNotify);
            PacketFuncDic.Add(PacketID.ResRoomLeave, PacketProcess_RoomLeaveResponse);
            PacketFuncDic.Add(PacketID.NtfRoomLeaveUser, PacketProcess_RoomLeaveUserNotify);
            PacketFuncDic.Add(PacketID.ResRoomChat, PacketProcess_RoomChatResponse);
            PacketFuncDic.Add(PacketID.NtfRoomChat, PacketProcess_RoomChatNotify);
            PacketFuncDic.Add(PacketID.ResReadyOmok, PacketProcess_ReadyOmokResponse);
            PacketFuncDic.Add(PacketID.NtfReadyOmok, PacketProcess_ReadyOmokNotify);
            PacketFuncDic.Add(PacketID.NtfStartOmok, PacketProcess_StartOmokNotify);
            PacketFuncDic.Add(PacketID.ResPutMok, PacketProcess_PutMokResponse);
            PacketFuncDic.Add(PacketID.NTFPutMok, PacketProcess_PutMokNotify);
            PacketFuncDic.Add(PacketID.NTFEndOmok, PacketProcess_EndOmokNotify);
        }

        void PacketProcess(PacketData packet)
        {
            var packetID = packet.PacketID;

            if (PacketFuncDic.ContainsKey(packetID))
            {
                PacketFuncDic[packetID](packet.BodyData);
            }
            else
            {
                DevLog.Write("Unknown Packet Id: " + packet.PacketID.ToString());
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




        void PacketProcess_ErrorNotify(byte[] bodyData)
        {
            /*var notifyPkt = new ErrorNtfPacket();
            notifyPkt.FromBytes(bodyData);

            DevLog.Write($"에러 통보 받음:  {notifyPkt.Error}");*/
        }


        void PacketProcess_Loginin(byte[] bodyData)
        {
            //var responsePkt = MessagePackSerializer.Deserialize<PKTResLogin>(packetData);
            //DevLog.Write($"로그인 결과: {(ErrorCode)responsePkt.Result}");
        }

        void PacketProcess_RoomEnterResponse(byte[] bodyData)
        {
            //var responsePkt = MessagePackSerializer.Deserialize<PKTResRoomEnter>(packetData);
            //DevLog.Write($"방 입장 결과:  {(ErrorCode)responsePkt.Result}");
        }

        void PacketProcess_RoomUserListNotify(byte[] bodyData)
        {
            //var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomUserList>(packetData);

            /*for (int i = 0; i < notifyPkt.UserIDList.Count; ++i)
            {
                AddRoomUserList(notifyPkt.UserIDList[i]);
            }*/

            DevLog.Write($"방의 기존 유저 리스트 받음");
        }

        void PacketProcess_RoomNewUserNotify(byte[] bodyData)
        {
            //var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomNewUser>(packetData);

            //AddRoomUserList(notifyPkt.UserID);

            DevLog.Write($"방에 새로 들어온 유저 받음");
        }


        void PacketProcess_RoomLeaveResponse(byte[] bodyData)
        {
            //var responsePkt = MessagePackSerializer.Deserialize<PKTResRoomLeave>(packetData);

            //DevLog.Write($"방 나가기 결과:  {(ErrorCode)responsePkt.Result}");
        }

        void PacketProcess_RoomLeaveUserNotify(byte[] bodyData)
        {
            //var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomLeaveUser>(packetData);

            //RemoveRoomUserList(notifyPkt.UserID);

            DevLog.Write($"방에서 나간 유저 받음");
        }


        void PacketProcess_RoomChatResponse(byte[] bodyData)
        {
            //var responsePkt = MessagePackSerializer.Deserialize<PKTResRoomChat>(packetData);

            //DevLog.Write($"방 채팅 결과:  {(ErrorCode)responsePkt.Result}");
        }


        void PacketProcess_RoomChatNotify(byte[] bodyData)
        {
            //var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomChat>(packetData);

            //AddRoomChatMessageList(notifyPkt.UserID, notifyPkt.ChatMessage);
        }

        void AddRoomChatMessageList(string userID, string message)
        {
            if (listBoxRoomChatMsg.Items.Count > 512)
            {
                listBoxRoomChatMsg.Items.Clear();
            }

            listBoxRoomChatMsg.Items.Add($"[{userID}]: {message}");
            listBoxRoomChatMsg.SelectedIndex = listBoxRoomChatMsg.Items.Count - 1;
        }

        void PacketProcess_ReadyOmokResponse(byte[] bodyData)
        {
            //var responsePkt = MessagePackSerializer.Deserialize<PKTResReadyOmok>(packetData);

            //DevLog.Write($"게임 준비 완료 요청 결과:  {(ErrorCode)responsePkt.Result}");
        }

        void PacketProcess_ReadyOmokNotify(byte[] bodyData)
        {
            //var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfReadyOmok>(packetData);

            /*if (notifyPkt.IsReady)
            {
                DevLog.Write($"[{notifyPkt.UserID}]님은 게임 준비 완료");
            }
            else
            {
                DevLog.Write($"[{notifyPkt.UserID}]님이 게임 준비 완료 취소");
            }*/

        }

        void PacketProcess_StartOmokNotify(byte[] bodyData)
        {
            /*var isMyTurn = false;

            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfStartOmok>(packetData);
            
            if(notifyPkt.FirstUserID == textBoxUserID.Text)
            {
                isMyTurn = true;
            }

            StartGame(isMyTurn, textBoxUserID.Text, GetOtherPlayer(textBoxUserID.Text));

            DevLog.Write($"게임 시작. 흑돌 플레이어: {notifyPkt.FirstUserID}");*/
        }
        

        void PacketProcess_PutMokResponse(byte[] bodyData)
        {
            //var responsePkt = MessagePackSerializer.Deserialize<PKTResPutMok>(packetData);

            //DevLog.Write($"오목 놓기 실패: {(ErrorCode)responsePkt.Result}");

            //TODO 방금 놓은 오목 정보를 취소 시켜야 한다
        }
        

        void PacketProcess_PutMokNotify(byte[] bodyData)
        {
            /*var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfPutMok>(packetData);

            플레이어_돌두기(true, notifyPkt.PosX, notifyPkt.PosY);

            DevLog.Write($"오목 정보: X: {notifyPkt.PosX},  Y: {notifyPkt.PosY},   알:{notifyPkt.Mok}");*/
        }
        

        void PacketProcess_EndOmokNotify(byte[] bodyData)
        {
            /*var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfEndOmok>(packetData);

            EndGame();

            DevLog.Write($"오목 GameOver: Win: {notifyPkt.WinUserID}");*/
        }
    }
}
