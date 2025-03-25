using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;
using Serilog;

namespace AutoTestClient.PacketHandler;

public class ResponseLogin : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        var responseData = MemoryPackSerializer.Deserialize<PKTResLogin>(packet);

        var errorCode = (CSCommon.ErrorCode)responseData.ErrorCode;
        if (errorCode != CSCommon.ErrorCode.None)
        {
            Log.Error($"[ResponseLogin] Failed login with error : {errorCode}");
            dummy.GetRunTimeData().AddError(errorCode, $"Failed login with error");

            return;
        }

        dummy.GetRunTimeData().SetState(DummyState.Login);
    }
}