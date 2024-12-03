using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class ResponseLogin : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            // 응답 확인
            var responseData = MemoryPackSerializer.Deserialize<PKTResLogin>(packet);
            var errorCode = (CSCommon.ErrorCode)responseData.ErrorCode;
            if (errorCode != CSCommon.ErrorCode.None)
            {
                Console.WriteLine($"Failed login with error : {errorCode}");
                dummy.ScenarioDone(false, $"Failed login with error : {errorCode}");

                Monitor.IncreaseFailedActionCount();
                return;
            }

            dummy.LoginSuccessAction();
        }
    }
}