using AutoTestClient.Scenario;

namespace AutoTestClient.Dummy
{
    public class DummyManager
    {
        public List<DummyObject> DummyList { get; private set; }

        public Dictionary<string, DummyObject> DummyDic { get; private set; }

        public void Init(ScenarioRunnerConfig config)
        {
            DummyList = new(config.DummyCount.Value);
            DummyDic = new(config.DummyCount.Value);

            var startNumber = config.DummyStartNumber.Value;

            for (int i = 0; i < DummyList.Capacity; ++i)
            {
                DummyObject dummy = new DummyObject();
                dummy.Init(i, startNumber + i, config);

                DummyList.Add(dummy);
                DummyDic.Add(dummy.ID, dummy);
            }
        }

        public async Task Action<T>(ScenarioRunnerConfig config) where T : ScenarioBase, new()
        {
            var tasks = new List<Task>();

            foreach (AutoTestClient.Dummy.DummyObject dummy in DummyList)
            {
                var scenario = new T();

                scenario.Init(config);
                tasks.Add(scenario.Action(dummy));
            }

            await Task.WhenAll(tasks.ToArray());
        }

        public DummyObject GetDummyByIndex(int index)
        {
            try
            {
                return DummyList[index];
            }
            catch { return null; }
        }

        public DummyObject GetDummyByID(string id)
        {
            try
            {
                return DummyDic[id];
            }
            catch { return null; }
        }

        public int GetSuccessedActionCount()
        {
            int sum = 0;
            foreach (DummyObject dummy in DummyList)
            {
                sum += dummy.SucceededActionCount;
            }

            return sum;
        }

        public double GetAvgRTT()
        {
            long sum = 0;
            foreach (DummyObject dummy in DummyList)
            {
                sum += dummy.RTT;
            }

            return sum / DummyList.Capacity;
        }

        public void CheckingActionTimeout()
        {
            foreach (DummyObject dummy in DummyList)
            {
                if (dummy.IsScenarioRunning == false)
                {
                    continue;
                }

                if (dummy.IsRunningActionTimeout() == true)
                {
                    dummy.ScenarioDone(false, "Action Timeout");
                }
            }
        }

        public void Receive()
        {
            foreach (DummyObject dummy in DummyList)
            {
                if (dummy.IsScenarioRunning == false)
                {
                    continue;
                }

                dummy.ReceiveAndAddPacketToPacketProcessor();
            }
        }
    }
}