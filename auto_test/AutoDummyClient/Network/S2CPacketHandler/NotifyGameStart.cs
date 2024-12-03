using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class NotifyGameStart : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            // 응답 확인
            var notifyData = MemoryPackSerializer.Deserialize<PKTNtfGameStart>(packet);

            // 동일한 방 유저가 보냈는지 확인
            var opponentID = notifyData.OpponentID;
            var opponentDummy = GetDummyByIDFunc(opponentID);
            if (dummy.EnteredRoomNumber != opponentDummy.EnteredRoomNumber)
            {
                Monitor.IncreaseFailedActionCount();

                Console.WriteLine($"Not match room user (me:{dummy.EnteredRoomNumber}, other:{opponentDummy.EnteredRoomNumber})");
                dummy.ScenarioDone(false, $"Not match room user (me:{dummy.EnteredRoomNumber}, other:{opponentDummy.EnteredRoomNumber})");
                return;
            }

            // 게임 정보 세팅
            var turn = notifyData.Turn;
            var stoneType = (StoneType)notifyData.StoneType;

            dummy.GameStartSuccessAction(turn, stoneType, opponentID);
        }
    }
}