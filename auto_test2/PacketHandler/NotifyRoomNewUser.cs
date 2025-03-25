using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;
using Serilog;

namespace AutoTestClient.PacketHandler;

public class NotifyRoomNewUser : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomNewUser>(packet);
        var enteredUserID = notifyData.UserID;

        Log.Debug($"[NotifyRoomNewUser] Dummy: {dummy.ID}");
    }
}