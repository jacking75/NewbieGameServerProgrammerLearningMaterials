using AutoTestClient.Dummy;

using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public class ResponseGameStart : BaseHandler
    {
        public override void Handle(DummyObject dummy, byte[] packet)
        {
            var responseData = MemoryPackSerializer.Deserialize<PKTResGameStart>(packet);

            // 응답 확인
            var errorCode = (CSCommon.ErrorCode)responseData.ErrorCode;
            if (errorCode != CSCommon.ErrorCode.None)
            {
                // 아직 다른 유저가 게임 시작을 하지 않았다.
                if (errorCode == CSCommon.ErrorCode.NotAllGameReady)
                {
                    // 게임 시작 요청 패킷에 대한 응답은 풀어둔다.
                    dummy.WakeForResponse();
                    return;
                }

                // 그게 아니면 에러다.
                Console.WriteLine($"Failed GameStart room with error : {errorCode}");
                dummy.ScenarioDone(false, $"Failed GameStart room with error : {errorCode}");

                Monitor.IncreaseFailedActionCount();
                return;
            }

            // 모든 유저가 게임을 시작했다.

            var turn = responseData.Turn;
            var stoneType = (StoneType)responseData.StoneType;
            var opponentID = responseData.OpponentID;
            dummy.GameStartSuccessAction(turn, stoneType, opponentID);
        }
    }
}