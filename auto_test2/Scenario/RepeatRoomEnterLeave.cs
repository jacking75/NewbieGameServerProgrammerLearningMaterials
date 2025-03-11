/*namespace AutoTestClient.Scenario
{
    public class RepeatRoomEnterLeave : ScenarioBase
    {
        public override async Task Action(AutoTestClient.Dummy.DummyObject dummy)
        {
            dummy.ScenarioPrepare(ScenarioType.RepeatRoomEnterLeave);

            // 1. 로그인
            var result1 = await ConnectLogin(dummy);
            if (result1 != ErrorCode.None)
            {
                dummy.ScenarioDone(false, $"Failed action(RepeatRoomEnterLeave) with error : {result1}");
                return;
            }

            // 2. 방 입장 -> 방 퇴장
            while (IsPossbleNextAction(dummy) == true)
            {
                var result2 = await RoomEnterLeave(dummy);

                if (result2 != ErrorCode.None)
                {
                    dummy.ScenarioDone(false, $"Failed action(RepeatRoomEnterLeave) with error : {result2}");
                    return;
                }

                dummy.SuccessScenarioAction();

                Monitor.IncreaseScenarioRepeacCount();
                Monitor.IncreaseScenarioRepeacCountPerSeconds();
            }

            dummy.ScenarioDone(true, $"Success RepeatRoomEnterLeave");
        }
    }
}
*/