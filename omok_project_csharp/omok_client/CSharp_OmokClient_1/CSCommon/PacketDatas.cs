using MessagePack;
using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{
    [MessagePackObject]
    public class PKTResponse : MsgPackPacketHead
    {
        [Key(1)]
        public short Result;
    }



    // 로그인 요청
    [MessagePackObject]
    public class PKTReqLogin : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;
        [Key(2)]
        public string AuthToken;
    }

    [MessagePackObject]
    public class PKTResLogin : PKTResponse
    {
    }



    [MessagePackObject]
    public class PKTNtfMustClose : PKTResponse
    {
    }



    // 방 입장
    [MessagePackObject]
    public class PKTReqRoomEnter : MsgPackPacketHead
    {
        [Key(1)]
        public int RoomNum;
    }
     
    [MessagePackObject]
    public class PKTResRoomEnter : PKTResponse
    {
    }

    [MessagePackObject]
    public class PKTNtfRoomUserList : MsgPackPacketHead
    {
        [Key(1)]
        public List<string> UserIDList = new List<string>();
    }

    [MessagePackObject]
    public class PKTNtfRoomNewUser : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;
    }


    // 방 나가기(보디가 없다)
    [MessagePackObject]
    public class PKTReqRoomLeave : MsgPackPacketHead
    {
    }

    [MessagePackObject]
    public class PKTResRoomLeave : PKTResponse
    {
    }

    [MessagePackObject]
    public class PKTNtfRoomLeaveUser : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;
    }


    [MessagePackObject]
    public class PKTReqRoomChat : MsgPackPacketHead
    {
        [Key(1)]
        public string ChatMessage;
    }

    [MessagePackObject]
    public class PKTResRoomChat : PKTResponse
    {
    }

    [MessagePackObject]
    public class PKTNtfRoomChat : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;

        [Key(2)]
        public string ChatMessage;
    }

    [MessagePackObject]
    public class PKTInternalReqRoomEnter : MsgPackPacketHead
    {
        [Key(1)]
        public int RoomNumber;

        [Key(2)]
        public string UserID;
    }


    // 오목 플레이 준비 완료 요청
    [MessagePackObject]
    public class PKTReqReadyOmok : PKTResponse
    {
    }

    [MessagePackObject]
    public class PKTResReadyOmok : PKTResponse
    {
    }

    [MessagePackObject]
    public class PKTNtfReadyOmok : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;
        [Key(2)]
        public bool IsReady;
    }


    // 오목 시작 통보(서버에서 클라이언트들에게)
    [MessagePackObject]
    public class PKTNtfStartOmok : MsgPackPacketHead
    {
        [Key(1)]
        public string FirstUserID; // 선턴 유저 ID
    }


    // 돌 두기
    [MessagePackObject]
    public class PKTReqPutMok : MsgPackPacketHead
    {
        [Key(1)]
        public int PosX;
        [Key(2)]
        public int PosY;        
    }

    [MessagePackObject]
    public class PKTResPutMok : PKTResponse
    {
    }

    [MessagePackObject]
    public class PKTNtfPutMok : MsgPackPacketHead
    {
        [Key(1)]
        public int PosX;
        [Key(2)]
        public int PosY;
        [Key(3)]
        public int Mok;
    }

    // 오목 게임 종료 통보
    [MessagePackObject]
    public class PKTNtfEndOmok : MsgPackPacketHead
    {
        [Key(1)]
        public string WinUserID;
    }

   
}
