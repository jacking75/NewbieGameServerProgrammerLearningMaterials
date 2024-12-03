using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class ResponsePutStone : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            var responseData = MemoryPackSerializer.Deserialize<PKTResPutStone>(packet);

            // 응답 확인
            var errorCode = (CSCommon.ErrorCode)responseData.ErrorCode;
            if (errorCode != CSCommon.ErrorCode.None)
            {
                Monitor.IncreaseFailedActionCount();
                dummy.ScenarioDone(false, $"Failed put stone with error : {errorCode}");
                return;
            }

            // 돌 놓기 결과 확인
            var gameResult = (돌놓기결과)responseData.GameResult;
            if (gameResult != 돌놓기결과.None)
            {
                // 게임이 종료됐다.
                if (gameResult == 돌놓기결과.GameDone)
                {
                    var winner = (StoneType)responseData.Winner;

                    // 여기서 게임을 끝내고 해당 더미를 깨우면, 다음 루프에서 게임 루프를 빠져나오게 된다.
                    dummy.GameDone(winner);
                    dummy.WakeForResponse();

                    Monitor.DecreaseGameDummyCount();
                    return;
                }

                // 돌이 이미 존재함.
                if (gameResult == 돌놓기결과.NotEmpty)
                {
                    Console.WriteLine("중복된 위치에 돌을 놨다.");
                    dummy.WakeForResponse();
                    return;
                }
            }

            // 돌 놓기만 성공했다면, 턴 변경.
            var turn = responseData.Turn;
            dummy.PutStoneSuccessAction(turn);
        }
    }
}