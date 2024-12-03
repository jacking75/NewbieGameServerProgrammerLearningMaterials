namespace AutoTestClient
{
    public class Monitor
    {
        public static long ConnectedDummyCount;
        public static void IncreaseConnectedDummyCount() => Interlocked.Increment(ref ConnectedDummyCount);
        public static void DecreaseConnectedDummyCount() => Interlocked.Decrement(ref ConnectedDummyCount);

        public static long LoginDummyCount;
        public static void IncreaseLoginDummyCount() => Interlocked.Increment(ref LoginDummyCount);
        public static void DecreaseLoginDummyCount() => Interlocked.Decrement(ref LoginDummyCount);

        public static long RoomDummyCount;
        public static void IncreaseRoomDummyCount() => Interlocked.Increment(ref RoomDummyCount);
        public static void DecreaseRoomDummyCount() => Interlocked.Decrement(ref RoomDummyCount);

        public static long GameDummyCount;
        public static void IncreaseGameDummyCount() => Interlocked.Increment(ref GameDummyCount);
        public static void DecreaseGameDummyCount() => Interlocked.Decrement(ref GameDummyCount);

        public static long FailedActionCount;
        public static void IncreaseFailedActionCount() => Interlocked.Increment(ref FailedActionCount);

        public static long WaitForResponsePacketCount;
        public static void IncreaseWaitForResponsePacketCount() => Interlocked.Increment(ref WaitForResponsePacketCount);
        public static void DecreaseWaitForResponsePacketCount() => Interlocked.Decrement(ref WaitForResponsePacketCount);

        public static long ScenarioRepeacCount;
        public static void IncreaseScenarioRepeacCount() => Interlocked.Increment(ref ScenarioRepeacCount);

        public static long ScenarioRepeacCountPerSeconds;
        public static void IncreaseScenarioRepeacCountPerSeconds() => Interlocked.Increment(ref ScenarioRepeacCountPerSeconds);


        public ScenarioType ScenarioType;
        public Func<int> GetPacketCountFunc;
        public Func<double> GetAvgRTTFunc;

        private bool _isRunning = false;
        private Thread _updater;
        private int _updateIntervalMilliSec;

        public void Init(int updateIntervalMilliSec)
        {
            _updateIntervalMilliSec = updateIntervalMilliSec;
            _updater = new Thread(Work);
        }

        public void Start()
        {
            _isRunning = true;
            _updater.Start();
        }

        public void Stop()
        {
            _isRunning = false;
            _updater.Join();
        }

        private void Work()
        {
            var startTime = DateTime.Now;

            while (_isRunning == true)
            {
                var now = DateTime.Now;
                var elpasedHours = (now - startTime).Hours;
                var elpasedMinutes = (now - startTime).Minutes;
                var elpasedSeconds = (now - startTime).Seconds;

                Console.WriteLine($"Running Scenario            : {ScenarioType}");
                Console.WriteLine($"Start Time                  : {startTime}");
                Console.WriteLine($"Elpased Time                : {elpasedHours}h {elpasedMinutes}m {elpasedSeconds}s");
                Console.WriteLine($"Sec Scenario Repeat Count   : {ScenarioRepeacCountPerSeconds}");
                Console.WriteLine($"Total Scenario Repeat Count : {ScenarioRepeacCount}");
                Console.WriteLine($"-------------------------------------");
                Console.WriteLine($"On Standby Packets Count    : {GetPacketCountFunc()}");
                Console.WriteLine($"Avg RTT                     : {GetAvgRTTFunc()} ms");
                Console.WriteLine($"-------------------------------------");
                Console.WriteLine($"Connected Dummy Count       : {ConnectedDummyCount}");
                Console.WriteLine($"Login Dummy     Count       : {LoginDummyCount}");
                Console.WriteLine($"Room Dummy      Count       : {RoomDummyCount}");
                Console.WriteLine($"Game Dummy      Count       : {GameDummyCount}");
                Console.WriteLine($"-------------------------------------");
                Console.WriteLine($"Response Packet Wait Count  : {WaitForResponsePacketCount}");
                Console.WriteLine($"Faeild Action Count         : {FailedActionCount}");
                Console.WriteLine($"-------------------------------------");
                Console.WriteLine($"\n\n\n");

                ScenarioRepeacCountPerSeconds = 0;

                Thread.Sleep(_updateIntervalMilliSec);
            }
        }
    }
}
