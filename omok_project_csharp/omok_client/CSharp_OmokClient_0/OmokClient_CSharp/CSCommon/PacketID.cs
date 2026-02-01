using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{
    // 1001 ~ 2000
    public class PacketID
    {
        public const UInt16 BEGIN = 1001;

        public const UInt16 ReqLogin = 1002;
        public const UInt16 ResLogin = 1003;

        public const UInt16 NtfMustClose = 1005;

        public const UInt16 ReqRoomEnter = 1015;
        public const UInt16 ResRoomEnter = 1016;
        public const UInt16 NtfRoomUserList = 1017;
        public const UInt16 NtfRoomNewUser = 1018;

        public const UInt16 ReqRoomLeave = 1021;
        public const UInt16 ResRoomLeave = 1022;
        public const UInt16 NtfRoomLeaveUser = 1023;

        public const UInt16 ReqRoomChat = 1026;
        public const UInt16 ResRoomChat = 1027;
        public const UInt16 NtfRoomChat = 1028;

        public const UInt16 ReqReadyOmok = 1031;
        public const UInt16 ResReadyOmok = 1032;
        public const UInt16 NtfReadyOmok = 1033;

        public const UInt16 NtfStartOmok = 1034;

        public const UInt16 ReqPutMok = 1035;
        public const UInt16 ResPutMok = 1036;
        public const UInt16 NTFPutMok = 1037;

        public const UInt16 NTFEndOmok = 1038;


        public const UInt16 END = 1100;
    }
}
