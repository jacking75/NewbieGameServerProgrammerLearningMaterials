using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;

namespace AutoTestClient.PacketHandler;

public class ResponseRoomEnter : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        var responseData = MemoryPackSerializer.Deserialize<PKTResRoomEnter>(packet);
        var errorCode = (CSCommon.ErrorCode)responseData.ErrorCode;
        if (errorCode != CSCommon.ErrorCode.None)
        {
            Console.WriteLine($"Failed EnterRoom with error : {errorCode}");
            dummy.GetRunTimeData().AddError(errorCode, $"Failed Enter Room with error");
            return;
        }

        dummy.GetRunTimeData().SetState(DummyState.Room);
    }
}