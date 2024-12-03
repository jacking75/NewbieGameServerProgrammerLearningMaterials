using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class NotifyPutStone : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            // 응답 확인
            var notifyData = MemoryPackSerializer.Deserialize<PKTNtfPutStone>(packet);
            var opponentID = notifyData.UserID;
            var opponentDummy = GetDummyByIDFunc(opponentID);

            // 동일한 방의 유저인가?
            if (dummy.EnteredRoomNumber != opponentDummy.EnteredRoomNumber)
            {
                Console.WriteLine($"Not match room user (me:{dummy.EnteredRoomNumber}, other:{opponentDummy.EnteredRoomNumber})");
                dummy.ScenarioDone(false, $"Not match room user (me:{dummy.EnteredRoomNumber}, other:{opponentDummy.EnteredRoomNumber})");

                Monitor.IncreaseFailedActionCount();
                return;
            }

            // 패킷 처리
            var errorCode = ProcessingNotifyData(notifyData, dummy);
            if (errorCode != ErrorCode.None)
            {
                Console.WriteLine($"Failed proccessing PutStone notify packet with error: {errorCode}");
                dummy.ScenarioDone(false, $"Failed proccessing PutStone notify packet with error: {errorCode}");

                Monitor.IncreaseFailedActionCount();
            }
        }

        private ErrorCode ProcessingNotifyData(PKTNtfPutStone notifyData, DummyObject dummy)
        {
            var stoneType = (StoneType)notifyData.StoneType;
            var x = notifyData.X;
            var y = notifyData.Y;
            var turn = notifyData.Turn;
            var winner = (StoneType)notifyData.Winner;
            var gameResult = (돌놓기결과)notifyData.GameResult;

            dummy.OmokBoard.돌놓기(stoneType, x, y);

            if (gameResult == 돌놓기결과.GameDone)
            {
                dummy.GameDone(winner);
                dummy.WakeForMyTurn();

                Monitor.DecreaseGameDummyCount();
                return ErrorCode.None;
            }

            dummy.WakeForMyTurn();

            return ErrorCode.None;
        }
    }
}

