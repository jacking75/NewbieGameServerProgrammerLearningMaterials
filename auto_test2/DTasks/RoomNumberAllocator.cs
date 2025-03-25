using System.Collections.Concurrent;

namespace AutoTestClient.DTasks;


public class RoomNumberAllocator
{
    static private ConcurrentQueue<Int32> _numbers = new();

    static public void Init(TestConfig config)
    {
        var count = config.MaxRoomCount;
        var startNumber = config.RoomStartNumber;

        for (Int32 i = 0; i < count; i++)
        {
            for (Int32 j = 0; j < config.RoomUserMaxCount; j++)
            {
                _numbers.Enqueue(startNumber + i);
            }
        }
    }

    static public Int32 Alloc()
    {
        if (_numbers.TryDequeue(out var roomNumber))
        {
            return roomNumber;
        }

        return -1;
    }

    static public void Release(Int32 roomNumber) => _numbers.Enqueue(roomNumber);

}

