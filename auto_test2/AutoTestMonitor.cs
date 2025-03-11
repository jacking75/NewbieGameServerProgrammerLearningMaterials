namespace AutoTestClient;

public class AutoTestMonitor
{
    private static readonly Lazy<AutoTestMonitor> _instance =
        new Lazy<AutoTestMonitor>(() => new AutoTestMonitor());
    public static AutoTestMonitor Instance => _instance.Value;


    DateTime _startTime;
    DateTime _endTime;

    private NetworkTrafficData _currentSecondData;
    private readonly Lock _lock = new();
    private List<NetworkTrafficData> _trafficHistory;
    private Timer _timer;
    private int _historyLength;


    public void Init(int keepHistorySeconds = 3600) // 기본 1시간치 데이터 보관
    {
        _historyLength = keepHistorySeconds;
        _currentSecondData = new NetworkTrafficData();
        _trafficHistory = new List<NetworkTrafficData>();

        // 1초마다 데이터 저장
        _timer = new Timer(SaveTrafficData, null, 0, 1000);
    }

    public void StartTest()
    {
        _startTime = DateTime.Now;
    }

    public void EndTest()
    {
        Dispose();
        _endTime = DateTime.Now;
    }


    public void RecordReceived(int bytes)
    {
        _currentSecondData.IncrementReceivedPackets();
        _currentSecondData.AddReceivedBytes(bytes);
    }

    public void RecordSent(int bytes)
    {
        _currentSecondData.IncrementSentPackets();
        _currentSecondData.AddSentBytes(bytes);
    }

    private void SaveTrafficData(object state)
    {
        lock (_lock)
        {
            _trafficHistory.Add(_currentSecondData);
            _currentSecondData = new NetworkTrafficData();

            if (_trafficHistory.Count > _historyLength)
            {
                _trafficHistory.RemoveAt(0);
            }
        }
    }

    public NetworkTrafficData GetCurrentTraffic()
    {
        var current = new NetworkTrafficData
        {
            ReceivedPackets = _currentSecondData.ReceivedPackets,
            SentPackets = _currentSecondData.SentPackets,
            ReceivedBytes = _currentSecondData.ReceivedBytes,
            SentBytes = _currentSecondData.SentBytes
        };
        return current;
    }

    public List<NetworkTrafficData> GetTrafficHistory()
    {
        lock (_lock)
        {
            return new List<NetworkTrafficData>(_trafficHistory);
        }
    }

    public NetworkTrafficData GetAverageTraffic(int lastSeconds)
    {
        lock (_lock)
        {
            if (lastSeconds <= 0 || _trafficHistory.Count == 0)
                return new NetworkTrafficData();

            int count = Math.Min(lastSeconds, _trafficHistory.Count);
            var relevantData = _trafficHistory.Skip(_trafficHistory.Count - count).Take(count);

            return new NetworkTrafficData
            {
                ReceivedPackets = (long)relevantData.Average(x => x.ReceivedPackets),
                SentPackets = (long)relevantData.Average(x => x.SentPackets),
                ReceivedBytes = (long)relevantData.Average(x => x.ReceivedBytes),
                SentBytes = (long)relevantData.Average(x => x.SentBytes)
            };
        }
    }

    public void Dispose()
    {
        _timer?.Dispose();
    }


}
