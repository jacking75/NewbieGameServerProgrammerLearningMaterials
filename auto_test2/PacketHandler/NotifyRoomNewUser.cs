using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;

namespace AutoTestClient.PacketHandler;

public class NotifyRoomNewUser : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        /*// 응답 확인
        var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomNewUser>(packet);
        var enteredUserID = notifyData.UserID;

        // 해당 더미에 새롭게 방에 입장한 유저 ID를 추가한다.
        var result = dummy.AddOtherUserID(enteredUserID);
        if (result != ErrorCode.None)
        {
            Console.WriteLine($"Failed added new room user with error: {result}");
            dummy.ScenarioDone(false, $"Failed added new room user with error: {result}");

            Monitor.IncreaseFailedActionCount();
        }*/
    }
}