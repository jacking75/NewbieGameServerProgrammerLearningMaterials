using CSCommon;
using MessagePack;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

#pragma warning disable CA1416

namespace OmokClient
{
    public partial class MainForm : Form
    {
        Dictionary<UInt16, Action<byte[]>> PacketFuncDic = new Dictionary<UInt16, Action<byte[]>>();

        void SetPacketHandler()
        {
            //PacketFuncDic.Add(PACKET_ID.PACKET_ID_ERROR_NTF, PacketProcess_ErrorNotify);
            PacketFuncDic.Add(PacketID.ResLogin, PacketProcess_Login);
            PacketFuncDic.Add(PacketID.ResRoomEnter, PacketProcess_RoomEnter);
            PacketFuncDic.Add(PacketID.NtfRoomUserList, PacketProcess_RoomUserList);
            PacketFuncDic.Add(PacketID.NtfRoomNewUser, PacketProcess_RoomNewUser);
            PacketFuncDic.Add(PacketID.ResRoomLeave, PacketProcess_RoomLeave);
            PacketFuncDic.Add(PacketID.NtfRoomLeaveUser, PacketProcess_RoomLeaveUser);
            PacketFuncDic.Add(PacketID.ResRoomChat, PacketProcess_RoomChatResponse);
            PacketFuncDic.Add(PacketID.NtfRoomChat, PacketProcess_RoomChatNotify);
            PacketFuncDic.Add(PacketID.ResReadyOmok, PacketProcess_ReadyOmokResponse);
            PacketFuncDic.Add(PacketID.NtfReadyOmok, PacketProcess_ReadyOmokNotify);
            PacketFuncDic.Add(PacketID.NtfStartOmok, PacketProcess_StartOmokNotify);
            PacketFuncDic.Add(PacketID.ResPutMok, PacketProcess_PutMokResponse);
            PacketFuncDic.Add(PacketID.NTFPutMok, PacketProcess_PutMokNotify);
            PacketFuncDic.Add(PacketID.NTFEndOmok, PacketProcess_EndOmokNotify);
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

        
        void PacketProcess_ErrorNotify(byte[] packetData)
        {
            /*var notifyPkt = new ErrorNtfPacket();
            notifyPkt.FromBytes(bodyData);

            DevLog.Write($"에러 통보 받음:  {notifyPkt.Error}");*/
        }


        void PacketProcess_Login(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResLogin>(packetData);
            DevLog.Write($"로그인 결과: {(ErrorCode)responsePkt.Result}");

            if ((ErrorCode)responsePkt.Result == ErrorCode.None)
            {
                CurSceen = ClientSceen.LOGIN;
            }
        }

        void PacketProcess_RoomEnter(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResRoomEnter>(packetData);
            DevLog.Write($"방 입장 결과:  {(ErrorCode)responsePkt.Result}");

            if ((ErrorCode)responsePkt.Result == ErrorCode.None)
            {
                CurSceen = ClientSceen.ROOM;
            }
        }

        void PacketProcess_RoomUserList(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomUserList>(packetData);

            for (int i = 0; i < notifyPkt.UserIDList.Count; ++i)
            {
                AddRoomUserList(notifyPkt.UserIDList[i]);
            }

            DevLog.Write($"방의 기존 유저 리스트 받음");
        }

        void PacketProcess_RoomNewUser(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomNewUser>(packetData);

            AddRoomUserList(notifyPkt.UserID);

            DevLog.Write($"방에 새로 들어온 유저 받음");
        }


        void PacketProcess_RoomLeave(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResRoomLeave>(packetData);

            DevLog.Write($"방 나가기 결과:  {(ErrorCode)responsePkt.Result}");
        }

        void PacketProcess_RoomLeaveUser(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomLeaveUser>(packetData);

            RemoveRoomUserList(notifyPkt.UserID);

            DevLog.Write($"방에서 나간 유저 받음");
        }


        void PacketProcess_RoomChatResponse(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResRoomChat>(packetData);

            DevLog.Write($"방 채팅 결과:  {(ErrorCode)responsePkt.Result}");
        }


        void PacketProcess_RoomChatNotify(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfRoomChat>(packetData);

            AddRoomChatMessageList(notifyPkt.UserID, notifyPkt.ChatMessage);
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

        void PacketProcess_ReadyOmokResponse(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResReadyOmok>(packetData);

            DevLog.Write($"게임 준비 완료 요청 결과:  {(ErrorCode)responsePkt.Result}");

            if ((ErrorCode)responsePkt.Result == ErrorCode.None)
            {
                CurSceen = ClientSceen.GAME_READY;
            }
        }

        void PacketProcess_ReadyOmokNotify(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfReadyOmok>(packetData);

            if (notifyPkt.IsReady)
            {
                DevLog.Write($"[{notifyPkt.UserID}]님은 게임 준비 완료");
            }
            else
            {
                DevLog.Write($"[{notifyPkt.UserID}]님이 게임 준비 완료 취소");
            }

        }

        void PacketProcess_StartOmokNotify(byte[] packetData)
        {
            IsMyTurn = false;

            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfStartOmok>(packetData);
            
            if(notifyPkt.BlackUserID == textBoxUserID.Text)
            {
                IsMyTurn = true;
            }
            
            CurSceen = ClientSceen.GAME_PLAYING;
            StartGame(IsMyTurn, notifyPkt.BlackUserID, notifyPkt.WhiteUserID);

            DevLog.Write($"게임 시작. 흑돌 플레이어: {notifyPkt.BlackUserID}");
        }
        

        void PacketProcess_PutMokResponse(byte[] packetData)
        {
            var responsePkt = MessagePackSerializer.Deserialize<PKTResPutMok>(packetData);

            if (responsePkt.Result != (Int16)ErrorCode.None)
            {
                DevLog.Write($"오목 놓기 성공");
            }
            else
            {
                DevLog.Write($"오목 놓기 실패: {(ErrorCode)responsePkt.Result}");
                OmokLogic.한수무르기();
            }            
        }
        

        void PacketProcess_PutMokNotify(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfPutMok>(packetData);

            플레이어_돌두기(IsMyTurn, notifyPkt.PosX, notifyPkt.PosY);

            IsMyTurn = !IsMyTurn;

            DevLog.Write($"오목 정보: X: {notifyPkt.PosX},  Y: {notifyPkt.PosY},   알:{notifyPkt.Mok}");
        }
        

        void PacketProcess_EndOmokNotify(byte[] packetData)
        {
            var notifyPkt = MessagePackSerializer.Deserialize<PKTNtfEndOmok>(packetData);

            EndGame();

            CurSceen = ClientSceen.ROOM;
            DevLog.Write($"오목 GameOver: Win: {notifyPkt.WinUserID}");
        }
    }
}
