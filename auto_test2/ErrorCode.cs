namespace AutoTestClient
{
    // 20000 ~ 24999
    public enum ErrorCode : Int16
    {
        None = 0,

        // Configs (20001 ~ 20500)
        InvalidScenario = 20001,
        InvalidScenarioRunTimeSec,
        InvalidScenarioRepeatCount,
        InvalidDummyCount,
        InvalidDummyStartNumber,
        InvalidDummyActionTimeoutSec,
        InvalidDummyActionIntervalMilliSec,
        InvalidRemoteEndPoint,
        InvalidRoomCount,
        InvalidRoomStartNumber,
        InvalidRoomUserMaxCount,

        // Login (20501 ~ 20600)
        UnableRequestLoginNotConnected = 20501,

        // Room (20801 ~ 20900)
        FailedRoomEnterIsFullRoom = 20801,
        FailedRoomEnterInvalidRoomNumber,
        FailedRoomLeaveInvalidRoomNumber,
        FailedRoomLeaveInvaildUserNumber,

        // RoomNumberAllocator (20901 ~ 20950)
        FailedRoomNumberAlloc = 20901,
        FailedRoomNumberReleaseInvaildRoomNumber,
        FailedRoomNumberReleaseRangeOver,

        // Socket (21001 ~ 21500)
        FailedConnect = 21001,
        FailedConnectTimeout,
        FailedDisconnect,
        FailedSend,
        FailedReceive,
        FailedReceivePacketTimeout,
        FailedReceiveRangeOverPacketSize,
        FailedReceivePacketException,
        FailedReceivePacketRecvBufferEmpty,
        FailedReceivePacketNotConnectedDummy,

        // etc
        FailedRequestProcess = 21501,
        SkipPacket = 21502,

        NotMatchOpponentID,
        NotMatchStoneType,
        NotMatchTurn,
        NotMatchGameResult,
        RangeOverOmokBoardX,
        RangeOverOmokBoardY,
        DuplicationX,
        DuplicationY,


        NotProccessingResponse,
        AlreadyRoomUserID,
        FailedOpenConfigFile = 24997,
        NotStartedAction = 24998,
        End = 24999,
    }
}