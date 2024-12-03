using AutoTestClient.Dummy;

namespace AutoTestClient.Scenario
{
    public class RepeatConnect : ScenarioBase
    {
        public override async Task Action(DummyObject dummy)
        {
            dummy.ScenarioPrepare(ScenarioType.RepeatConnect);

            while (IsPossbleNextAction(dummy) == true)
            {
                var result = await ConnectDisconnect(dummy);

                if (result != ErrorCode.None)
                {
                    dummy.ScenarioDone(false, $"Failed action(RepeatConnect) with error : {result}");
                    return;
                }

                dummy.SuccessScenarioAction();

                Monitor.IncreaseScenarioRepeacCount();
                Monitor.IncreaseScenarioRepeacCountPerSeconds();
            }

            dummy.ScenarioDone(true, "Success RepeatConnect");
        }
    }
}
