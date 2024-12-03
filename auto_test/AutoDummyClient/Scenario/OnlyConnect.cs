namespace AutoTestClient.Scenario
{
    public class OnlyConnect : ScenarioBase
    {
        public override async Task Action(AutoTestClient.Dummy.DummyObject dummy)
        {
            dummy.ScenarioPrepare(ScenarioType.OnlyConnect);

            while (IsPossbleNextAction(dummy) == true)
            {
                var result = await dummy.ConnectAction();

                if (result != ErrorCode.None)
                {
                    dummy.ScenarioDone(false, $"Failed action(OnlyConnect) with error : {result}");
                    return;
                }

                dummy.SuccessScenarioAction();

                Monitor.IncreaseScenarioRepeacCount();
                Monitor.IncreaseScenarioRepeacCountPerSeconds();
            }

            dummy.ScenarioDone(true, "Success OnlyConnect");
        }
    }
}

