using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{

    // 1000 ~ 19999
    public enum ErrorCode : UInt16
    {
        None                                = 0,
        
        GWLoginRedisCantFound             = 1021,
        GWLoginRedisAlwaysCurState = 1022,

        LobbyEnterDisableEnter = 1031,
        /*LoginFail                           = 1042,
        LoginAlreadyLogin                   = 1043,

        LobbyEnterLobbyOverflow             = 1050,
        LobbyEnterInvaildUser               = 1051,

        RoomLeaveFail                       = 1060,*/
    }
}
