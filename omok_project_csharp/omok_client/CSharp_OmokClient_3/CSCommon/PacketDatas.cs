using MessagePack;
using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{
    [MessagePackObject]
    public class PKNtfMustClose : MsgPackPacketHead
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
    public class PKTResLogin : MsgPackPacketHead
    {
        [Key(1)]
        public short Result;
    }

        

    [MessagePackObject]
    public class PKTReqLobbyEnter : MsgPackPacketHead
    {
        [Key(1)]
        public Int16 LobbyNumber;
    }

    [MessagePackObject]
    public class PKTResLobbyEnter : MsgPackPacketHead
    {
        [Key(1)]
        public short Result;
        [Key(2)]
        public Int16 LobbyNumber;
    }

    [MessagePackObject]
    public class PKTNtfLobbyEnterNewUser : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;
    }


    [MessagePackObject]
    public class PKTResLobbyLeave : MsgPackPacketHead
    {
        [Key(1)]
        public short Result;
    }

    [MessagePackObject]
    public class PKTNtfLobbyLeaveUser : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;
    }


    [MessagePackObject]
    public class PKTReqLobbyChat : MsgPackPacketHead
    {
        [Key(1)]
        public string Message;
    }


    [MessagePackObject]
    public class PKTNtfLobbyChat : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;

        [Key(2)]
        public string Message;
    }




    [MessagePackObject]
    public class PKTReqRoomEnter : MsgPackPacketHead
    {
        [Key(1)]
        public int RoomNumber;
    }

    [MessagePackObject]
    public class PKTResRoomEnter : MsgPackPacketHead
    {
        [Key(1)]
        public short Result;
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


    [MessagePackObject]
    public class PKTReqRoomLeave : MsgPackPacketHead
    {
    }

    [MessagePackObject]
    public class PKTResRoomLeave : MsgPackPacketHead
    {
        [Key(1)]
        public short Result;
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
    public class PKTNtfRoomChat : MsgPackPacketHead
    {
        [Key(1)]
        public string UserID;

        [Key(2)]
        public string ChatMessage;
    }
}
