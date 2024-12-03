namespace AutoTestClient
{
    public class ScenarioRunnerConfig
    {
        public ConfigData<Int32> Scenario { get; set; }

        public ConfigData<Int32> ScenarioRunTimeSec { get; set; }

        public ConfigData<Int32> ScenarioRepeatCount { get; set; }

        public ConfigData<Int32> DummyCount { get; set; }

        public ConfigData<Int32> DummyStartNumber { get; set; }

        public ConfigData<Int32> DummyActionIntervalMilliSec { get; set; }

        public ConfigData<Int32> DummyActionTimeoutSec { get; set; }

        public ConfigData<string> RemoteEndPoint { get; set; }

        public ConfigData<Int32> RoomCount { get; set; }

        public ConfigData<Int32> RoomStartNumber { get; set; }

        public ConfigData<Int32> RoomUserMaxCount { get; set; }

        public ErrorCode Verify()
        {
            if (Scenario.Value <= (Int32)ScenarioType.NONE || Scenario.Value >= (Int32)ScenarioType.END)
            {
                return ErrorCode.InvalidScenario;
            }

            bool flag = false;
            if (ScenarioRunTimeSec.Value < 0)
            {
                return ErrorCode.InvalidScenarioRunTimeSec;
            }
            else
            {
                if (ScenarioRunTimeSec.Value != 0)
                {
                    flag = true;
                    if (ScenarioRepeatCount.Value != 0)
                    {
                        return ErrorCode.InvalidScenarioRepeatCount;
                    }
                }
            }

            if (flag == false)
            {
                if (ScenarioRepeatCount.Value < 0)
                {
                    return ErrorCode.InvalidScenarioRepeatCount;
                }
                else
                {
                    if (ScenarioRunTimeSec.Value != 0)
                    {
                        return ErrorCode.InvalidScenarioRunTimeSec;
                    }
                }
            }

            if (DummyCount.Value < 1)
            {
                return ErrorCode.InvalidDummyCount;
            }

            if (DummyStartNumber.Value < 0)
            {
                return ErrorCode.InvalidDummyStartNumber;
            }

            if (DummyActionTimeoutSec.Value < 0)
            {
                return ErrorCode.InvalidDummyActionTimeoutSec;
            }

            if (DummyActionIntervalMilliSec.Value < 0)
            {
                return ErrorCode.InvalidDummyActionIntervalMilliSec;
            }

            if (string.IsNullOrEmpty(RemoteEndPoint.Value) == true)
            {
                return ErrorCode.InvalidRemoteEndPoint;
            }

            if (IsRoomScenario() == true)
            {
                if (RoomCount.Value < 1)
                {
                    return ErrorCode.InvalidRoomCount;
                }

                if (RoomStartNumber.Value < 0)
                {
                    return ErrorCode.InvalidRoomStartNumber;
                }

                if (RoomUserMaxCount.Value < 0)
                {
                    return ErrorCode.InvalidRoomUserMaxCount;
                }
            }

            return ErrorCode.None;
        }

        public bool IsRoomScenario()
        {
            if (Scenario.Value >= (Int32)ScenarioType.RepeatRoomEnterLeave)
            {
                return true;
            }
            return false;
        }
    }

    public class ConfigData<T>
    {
        public T Value { get; set; }

        public string Comment { get; set; }
    }
}
