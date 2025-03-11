using System;
using System.Collections.Generic;

using MemoryPack;

namespace CSCommon
{
    [MemoryPackable]
    public partial class PKTReqLogin : PacketHead
    {
        public string UserID;
        public string AuthToken;
    }

    [MemoryPackable]
    public partial class PKTResLogin : PacketHead
    {
        public Int16 ErrorCode;
    }

    [MemoryPackable]
    public partial class PKTReqRoomEnter : PacketHead
    {
        public Int32 RoomNumber;
    }

    [MemoryPackable]
    public partial class PKTResRoomEnter : PacketHead
    {
        public Int16 ErrorCode;
    }

    [MemoryPackable]
    public partial class PKTNtfRoomUserList : PacketHead
    {
        public List<string> UserIDList;
    }

    [MemoryPackable]
    public partial class PKTNtfRoomNewUser : PacketHead
    {
        public string UserID;
    }

    [MemoryPackable]
    public partial class PKTReqRoomLeave : PacketHead
    {
    }

    [MemoryPackable]
    public partial class PKTResRoomLeave : PacketHead
    {
        public Int16 ErrorCode;
    }

    [MemoryPackable]
    public partial class PKTNtfRoomLeaveUser : PacketHead
    {
        public string UserID;
    }

    [MemoryPackable]
    public partial class PKTReqRoomChat : PacketHead
    {
        public string ChatMessage;
    }

    [MemoryPackable]
    public partial class PKTNtfRoomChat : PacketHead
    {
        public string UserID;
        public string ChatMessage;
    }

    [MemoryPackable]
    public partial class PKTReqGameStart : PacketHead
    {
    }

    [MemoryPackable]
    public partial class PKTResGameStart : PacketHead
    {
        public Int16 ErrorCode;
        public bool Turn;
        public byte StoneType;
        public string OpponentID;
    }

    [MemoryPackable]
    public partial class PKTNtfGameStart : PacketHead
    {
        public bool Turn;
        public byte StoneType;
        public string OpponentID;
    }

    [MemoryPackable]
    public partial class PKTReqPutStone : PacketHead
    {
        public Int32 X;
        public Int32 Y;
    }

    [MemoryPackable]
    public partial class PKTResPutStone : PacketHead
    {
        public Int16 ErrorCode;

        public Int16 GameResult;
        public bool Turn;
        public byte Winner;
    }

    [MemoryPackable]
    public partial class PKTNtfPutStone : PacketHead
    {
        public Int16 GameResult;
        public bool Turn;
        public byte Winner;

        public Int32 X;
        public Int32 Y;
        public string UserID;
        public byte StoneType;
    }
}


