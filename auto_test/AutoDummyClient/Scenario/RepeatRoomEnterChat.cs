using AutoTestClient.Dummy;

namespace AutoTestClient.Scenario
{
    public class RepeatRoomEnterChat : ScenarioBase
    {
        public override async Task Action(DummyObject dummy)
        {
            dummy.ScenarioPrepare(ScenarioType.RepeatRoomEnterChat);

            // 1. 방 입장
            var result1 = await ConnectLoginRoomEnter(dummy);
            if (result1 != ErrorCode.None)
            {
                dummy.ScenarioDone(false, $"Failed action(RepeatRoomEnterLeave) with error : {result1}");
                return;
            }

            // 2. 방 채팅
            while (IsPossbleNextAction(dummy) == true)
            {
                var result2 = await dummy.RoomChatRequestAction();

                if (result2 != ErrorCode.None)
                {
                    dummy.ScenarioDone(false, $"Failed action(RepeatRoomEnterChat) with error : {result2}");
                    return;
                }

                dummy.SuccessScenarioAction();

                Monitor.IncreaseScenarioRepeacCount();
                Monitor.IncreaseScenarioRepeacCountPerSeconds();
            }

            dummy.ScenarioDone(true, $"Success RepeatRoomEnterChat");
        }
    }
}

