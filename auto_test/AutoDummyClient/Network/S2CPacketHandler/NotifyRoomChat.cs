using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class NotifyRoomChat : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            // 응답 확인
            var notifyData = MemoryPackSerializer.Deserialize<PKTNtfRoomChat>(packet);
            var senderID = notifyData.UserID;
            var senderDummy = GetDummyByIDFunc(senderID);
            var chatMessage = notifyData.ChatMessage;

            // 존재하는 더미인가?
            if (senderDummy is null)
            {
                Monitor.IncreaseFailedActionCount();
                Console.WriteLine($"Invalid senderID: {senderID}");

                dummy.ScenarioDone(false, $"Invalid senderID: {senderID}");
                return;
            }

            // 동일한 방 더미가 보낸 메시지인가?
            if (dummy.EnteredRoomNumber != senderDummy.EnteredRoomNumber)
            {
                Monitor.IncreaseFailedActionCount();
                Console.WriteLine($"Not in the room dummy (ThisRoomNumber: {dummy.EnteredRoomNumber}, DummyIndex: {senderDummy.Index}, EnteredRoomNumber: {senderDummy.EnteredRoomNumber}");

                dummy.ScenarioDone(false, $"Not in the room dummy (ThisRoomNumber: {dummy.EnteredRoomNumber}, DummyIndex: {senderDummy.Index}, EnteredRoomNumber: {senderDummy.EnteredRoomNumber}");
                return;
            }

            // 채팅을 보낸 더미가 자신인 경우 아래 처리를 진행해야한다.
            if (dummy.Index == senderDummy.Index)
            {
                // 해당 더미가 송신했던 메시지인가?
                if (string.Compare(dummy.SendedChatMessage, chatMessage) != 0)
                {
                    Monitor.IncreaseFailedActionCount();
                    Console.WriteLine($"Not match chat message: (SendedMessage: {dummy.SendedChatMessage}, ReceivedChatMessage: {chatMessage})");

                    dummy.ScenarioDone(false, $"Not match chat message: (SendedMessage: {dummy.SendedChatMessage}, ReceivedChatMessage: {chatMessage})");
                    return;
                }

                // 대기 중인 더미를 깨워주자.
                dummy.RoomChatSuccessAction();
            }
        }
    }
}