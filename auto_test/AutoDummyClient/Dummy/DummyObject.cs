namespace AutoTestClient.Dummy
{
    public class DummyObject : DummyAction
    {
        public void Init(Int32 index, Int32 number, ScenarioRunnerConfig config)
        {
            Index = index;
            Number = number;
            ID = $"DUMMY_{number}";

            if (config.IsRoomScenario() == true)
            {
                OtherUserIDList = new(config.RoomUserMaxCount.Value);
            }

            InitNetworkConfig(config);
            InitActionConfig(config);
            InitGameConfig();
        }
    }
}
