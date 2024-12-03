using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class ResponseRoomLeave : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            // 응답 확인
            var responseData = MemoryPackSerializer.Deserialize<PKTResRoomLeave>(packet);
            var errorCode = (CSCommon.ErrorCode)responseData.ErrorCode;
            if (errorCode != CSCommon.ErrorCode.None)
            {
                Monitor.IncreaseFailedActionCount();

                dummy.ScenarioDone(false, $"Failed RoomLeave with error : {errorCode}");
                return;
            }

            dummy.RoomLeaveSuccessAction();
        }
    }
}