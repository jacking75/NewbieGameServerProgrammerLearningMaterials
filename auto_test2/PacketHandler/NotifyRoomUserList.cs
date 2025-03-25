using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;

namespace AutoTestClient.PacketHandler;

public class NotifyRoomUserList : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        //var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomUserList>(packet);
        
    }
}