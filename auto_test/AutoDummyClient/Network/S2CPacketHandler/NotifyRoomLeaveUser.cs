using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;
namespace AutoTestClient.Network.S2CPacketHandler
{
    public class NotifyRoomLeaveUser : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            // 응답 확인
            var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomLeaveUser>(packet);
            var leavedUserID = notifyData.UserID;

            // 나간 유저 ID 삭제 (동일한 방 유저가 아니면 함수에 실패한다.)
            if (dummy.RemoveOtherUserID(leavedUserID) == false)
            {
                Console.WriteLine($"Failed remove room user (RemovedUserID: {leavedUserID}");
                dummy.ScenarioDone(false, $"Failed remove room user (RemovedUserID: {leavedUserID}");

                Monitor.IncreaseFailedActionCount();
            }
        }
    }
}