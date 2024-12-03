using AutoTestClient.Dummy;
using AutoTestClient.Network;
using AutoTestClient.Network.S2CPacketHandler;
using AutoTestClient.Scenario;

namespace AutoTestClient
{
    public class ScenarioRunner
    {
        private readonly Dummy.DummyManager _dummyMgr = new();
        private readonly PacketProcessor _packetProcessor = new();
        private readonly Receiver _receiver = new();
        private readonly Monitor _monitor = new();
        private readonly ActionTimeChecker _actionTimeChecker = new();
        private readonly RoomNumberAllocator _roomNumberAllocator = new();

        private ScenarioRunnerConfig _config;
        private DateTime _startTime;
        private DateTime _endTime;

        private const Int32 ReceiverUpdateIntervalMilliSec = 50;
        private const Int32 MonitorUpdateIntervalMilliSec = 999;
        private const Int32 ActionTimeCheckerUpdateIntervalMilliSec = 999;

        public void Init(ScenarioRunnerConfig config)
        {
            _config = config;

            // Initial Dummy & DummyManager 
            DummyObject.AddPacketToPacketProcessorFunc = _packetProcessor.Add;
            if (config.IsRoomScenario() == true)
            {
                _roomNumberAllocator.Init(config);

                DummyObject.AllocRoomNumberFunc = _roomNumberAllocator.Alloc;
                DummyObject.ReleaseRoomNumberFunc = _roomNumberAllocator.Release;
            }

            _dummyMgr.Init(config);

            // Initial PacketProcessor
            BaseHandler.GetDummyByIndexFunc = _dummyMgr.GetDummyByIndex;
            BaseHandler.GetDummyByIDFunc = _dummyMgr.GetDummyByID;
            _packetProcessor.GetDummyByIndexFunc = _dummyMgr.GetDummyByIndex;
            _packetProcessor.Init();

            // Initial Receiver
            _receiver.ReceiveAction = _dummyMgr.Receive;
            _receiver.Init(ReceiverUpdateIntervalMilliSec);

            // Initial Monitor
            _monitor.ScenarioType = (ScenarioType)config.Scenario.Value;
            _monitor.GetPacketCountFunc = _packetProcessor.GetPacketCount;
            _monitor.GetAvgRTTFunc = _dummyMgr.GetAvgRTT;
            _monitor.Init(MonitorUpdateIntervalMilliSec);

            // Initial ActionTimeChecker
            _actionTimeChecker.CheckingAction = _dummyMgr.CheckingActionTimeout;
            _actionTimeChecker.Init(ActionTimeCheckerUpdateIntervalMilliSec);
        }

        public void Run()
        {
            Prepare();

            switch ((ScenarioType)_config.Scenario.Value)
            {
                case ScenarioType.OnlyConnect:
                    _dummyMgr.Action<OnlyConnect>(_config).Wait();
                    break;

                case ScenarioType.RepeatConnect:
                    _dummyMgr.Action<RepeatConnect>(_config).Wait();
                    break;

                case ScenarioType.RepeatLogin:
                    _dummyMgr.Action<RepeatLogin>(_config).Wait();
                    break;

                case ScenarioType.RepeatRoomEnterLeave:
                    _dummyMgr.Action<RepeatRoomEnterLeave>(_config).Wait();
                    break;

                case ScenarioType.RepeatRoomEnterChat:
                    _dummyMgr.Action<RepeatRoomEnterChat>(_config).Wait();
                    break;

                case ScenarioType.OnlyPlayGame:
                    _dummyMgr.Action<OnlyPlayGame>(_config).Wait();
                    break;

                default:
                    break;
            }

            Done();

            Record();
        }

        private void Record()
        {
            var totalActionCount = _dummyMgr.GetSuccessedActionCount();
            var failedCount = 0;
            var succeededCount = 0;
            var elapsedTime = _endTime - _startTime;

            Console.WriteLine($"--------------------------------------------------------------------------------");
            Console.WriteLine($"[{DateTime.Now}]");
            foreach (var dummy in _dummyMgr.DummyList)
            {
                var result = dummy.ScenarioResult;

                if (result.IsSucceeded == false)
                {
                    ++failedCount;
                }
                else
                {
                    ++succeededCount;
                }

                Console.WriteLine($"Number: {dummy.Number}, IsSucceeded: {result.IsSucceeded}, Message: {result.Message}");
            }

            Console.WriteLine($"\nScenarioType: {_config.Scenario.Value} ElapsedTime: {elapsedTime.TotalMilliseconds}ms, TotalActionCount: {totalActionCount}, Succeeded Dummy Count: {succeededCount}, Failed Dummy Count: {failedCount}");
            Console.WriteLine($"--------------------------------------------------------------------------------");
        }

        private void Prepare()
        {
            _startTime = DateTime.Now;

            _monitor.Start();

            _packetProcessor.Start();

            _receiver.Start();

            _actionTimeChecker.Start();
        }

        private void Done()
        {
            _actionTimeChecker.Stop();

            _receiver.Stop();

            _packetProcessor.Stop();

            _monitor.Stop();

            _endTime = DateTime.Now;
        }
    }
}
