/*namespace AutoTestClient.Scenario
{
    public class OnlyPlayGame : ScenarioBase
    {
        public override async Task Action(AutoTestClient.Dummy.DummyObject dummy)
        {
            dummy.ScenarioPrepare(ScenarioType.RepeatRoomEnterLeave);

            // 1. 접속 -> 로그인 -> 방 입장
            var result1 = await ConnectLoginRoomEnter(dummy);
            if (result1 != ErrorCode.None)
            {
                dummy.ScenarioDone(false, $"Failed action(OnlyPlayGame) with error : {result1}");
                return;
            }

            while (IsPossbleNextAction(dummy) == true)
            {
                // 2.게임 시작 대기
                var result2 = await GameStart(dummy);
                if (result2 != ErrorCode.None)
                {
                    dummy.ScenarioDone(false, $"Failed action(OnlyPlayGame) with error : {result2}");
                    return;
                }

                // 3. 게임 진행
                var result3 = await PlayGame(dummy);
                if (result3 != ErrorCode.None)
                {
                    dummy.ScenarioDone(false, $"Failed action(OnlyPlayGame) with error : {result3}");
                    return;
                }

                dummy.SuccessScenarioAction();

                Monitor.IncreaseScenarioRepeacCount();
                Monitor.IncreaseScenarioRepeacCountPerSeconds();
            }

            // 4. 방 나가기
            var result4 = await dummy.RoomLeaveRequestAction();
            if (result4 != ErrorCode.None)
            {
                dummy.ScenarioDone(false, $"Failed action(OnlyPlayGame) with error : {result4}");
                return;
            }

            dummy.ScenarioDone(true, $"Success OnlyPlayGame (Stone:{dummy.StoneType}, Win:{dummy.IsWinner}, OpponentID:{dummy.OpponentID})");
        }
    }
}

*/