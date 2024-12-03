using System;

namespace CSCommon
{
    // 101 ~ 500
    public enum PacketID : Int16
    {
        BEGIN = 101,

        ReqLogin = 111,
        ResLogin = 112,

        ReqRoomEnter = 211,
        ResRoomEnter = 212,
        NtfRoomNewUser = 213,
        NtfRoomUserList = 214,

        ReqRoomLeave = 215,
        ResRoomLeave = 216,
        NtfRoomLeaveUser = 217,

        ReqRoomChat = 225,
        NtfRoomChat = 226,

        ReqGameStart = 311,
        ResGameStart = 312,
        NtfGameStart = 313,

        ReqPutStone = 321,
        ResPutStone = 322,
        NtfPutStone = 323,

        InnerConnectedSession = 401,
        InnerClosedSession = 402,

        END = 500,
    }
}