using AutoTestClient.Dummy;
using CSCommon;

using MemoryPack;

namespace AutoTestClient.PacketHandler;

public class NotifyRoomChat : BaseHandler
{
    public override void Handle(DummyObject dummy, byte[] packet)
    {
        var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomChat>(packet);
        var senderID = notifyData.UserID;
        var senderDummy = GetDummyByIDFunc(senderID);
        var chatMessage = notifyData.ChatMessage;

        if(dummy.ID == senderID)
        {
            dummy.GetRunTimeData().CheckChatMessage(chatMessage);
        }
    }
}