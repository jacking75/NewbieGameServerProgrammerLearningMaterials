using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{
    // Client - GatewayServer PacketID 정의 
    // 101 ~ 500
    public class PacketID
    {
        public const UInt16 BEGIN = 101;


        public const UInt16 NtfMustClose = 102;

        public const UInt16 ReqLogin = 111;
        public const UInt16 ResLogin = 112;

        public const UInt16 ReqLobbyEnter = 121;
        public const UInt16 ResLobbyEnter = 122;
        public const UInt16 NtfLobbyEnterNewUser = 123;

        public const UInt16 ReqLobbyLeave = 126;
        public const UInt16 ResLobbyLeave = 127;
        public const UInt16 NtfLobbyLeaveUser = 128;

        public const UInt16 RelayLobbyBegin = 131;
        public const UInt16 ReqLobbyChat = 136;
        public const UInt16 NtfLobbyChat = 137;
        public const UInt16 RelayLobbyEnd = 171;




        public const UInt16 ReqRoomEnter = 211;
        public const UInt16 ResRoomEnter = 212;

        public const UInt16 ReqRoomLeave = 216;
        public const UInt16 ResRoomLeave = 217;


        public const UInt16 NTFGameInfo = 251;
        public const UInt16 ReqGamePut = 252;
        public const UInt16 NTFGamePut = 253;
        public const UInt16 NTFGameResult = 256;


        public const UInt16 END = 500;
    }
}
