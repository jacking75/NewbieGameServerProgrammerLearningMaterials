using System.Collections.Concurrent;

namespace AutoTestClient
{
    public class RoomNumberAllocator
    {
        private ConcurrentQueue<Int32> _numbers = new();

        public void Init(ScenarioRunnerConfig config)
        {
            var count = config.RoomCount.Value;
            var startNumber = config.RoomStartNumber.Value;
            for (Int32 i = 0; i < count; i++)
            {
                for (Int32 j = 0; j < config.RoomUserMaxCount.Value; j++)
                {
                    _numbers.Enqueue(startNumber + i);
                }
            }
        }

        public Int32 Alloc()
        {
            Int32 roomNumber;

            if (_numbers.TryDequeue(out roomNumber) == false)
            {
                return -1;
            }

            return roomNumber;
        }

        public void Release(Int32 roomNumber) => _numbers.Enqueue(roomNumber);
    }
}

