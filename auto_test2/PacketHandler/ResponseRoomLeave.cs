using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;
using Serilog;

namespace AutoTestClient.PacketHandler;

public class ResponseRoomLeave : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        var responseData = MemoryPackSerializer.Deserialize<PKTResRoomLeave>(packet);

        var errorCode = (CSCommon.ErrorCode)responseData.ErrorCode;
        if (errorCode != CSCommon.ErrorCode.None)
        {
            Log.Error($"[ResponseRoomLeave] Failed LeaveRoom with error : {errorCode}");

            dummy.GetRunTimeData().AddError(errorCode, $"Failed LeaveRoom with error");
            return;
        }

        dummy.GetRunTimeData().SetState(DummyState.Login);
    }
}