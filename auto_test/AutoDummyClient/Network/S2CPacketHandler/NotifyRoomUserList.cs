using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class NotifyRoomUserList : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            // 응답 확인
            var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomUserList>(packet);
            var otherUserIDList = notifyData.UserIDList;

            // 방에 입장한 더미는 기존에 방에 존재했던 유저(들)의 ID를 추가한다.
            foreach (var otherUserID in otherUserIDList)
            {
                var result = dummy.AddOtherUserID(otherUserID);

                if (result != ErrorCode.None)
                {
                    Console.WriteLine($"Failed Add other user id with error : {result}");
                    dummy.ScenarioDone(false, message: $"Failed Add other user id with error : {result}");

                    Monitor.IncreaseFailedActionCount();
                    return;
                }
            }
        }
    }
}