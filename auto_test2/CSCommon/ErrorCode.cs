using System;

namespace CSCommon
{
    // 1000 ~ 19999
    public enum ErrorCode : Int16
    {
        None = 0,
        AlreadyLoginState = 1001,
        AlreadyRoomEnterState,
        InvalidRoomNumber,
        NotRoomUser,
        FullRoom,
        NotAllGameReady,
    }
}