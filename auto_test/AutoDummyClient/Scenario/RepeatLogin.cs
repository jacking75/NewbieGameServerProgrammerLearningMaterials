namespace AutoTestClient.Scenario
{
    public class RepeatLogin : ScenarioBase
    {
        public override async Task Action(AutoTestClient.Dummy.DummyObject dummy)
        {
            dummy.ScenarioPrepare(ScenarioType.RepeatLogin);

            while (IsPossbleNextAction(dummy) == true)
            {
                var result = await ConnectLoginDisconnect(dummy);

                if (result != ErrorCode.None)
                {
                    dummy.ScenarioDone(false, $"Failed action(RepeatLogin) with error : {result}");
                    return;
                }

                dummy.SuccessScenarioAction();

                Monitor.IncreaseScenarioRepeacCount();
                Monitor.IncreaseScenarioRepeacCountPerSeconds();
            }

            dummy.ScenarioDone(true, "Success RepeatLogin");
        }
    }
}

