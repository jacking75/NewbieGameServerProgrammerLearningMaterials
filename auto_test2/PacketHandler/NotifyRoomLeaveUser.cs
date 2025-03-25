using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;
using Serilog;
namespace AutoTestClient.PacketHandler;

public class NotifyRoomLeaveUser : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomLeaveUser>(packet);
        var leavedUserID = notifyData.UserID;

        Log.Debug($"[NotifyRoomLeaveUser] Dummy: {dummy.ID}");
    }
}