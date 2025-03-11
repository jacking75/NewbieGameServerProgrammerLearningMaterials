using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient;

public class NetworkTrafficData
{
    private long _receivedPackets;
    private long _sentPackets;
    private long _receivedBytes;
    private long _sentBytes;
    public DateTime Timestamp { get; private set; }

    public NetworkTrafficData()
    {
        Timestamp = DateTime.Now;
    }

    public long ReceivedPackets
    {
        get => Interlocked.Read(ref _receivedPackets);
        set => Interlocked.Exchange(ref _receivedPackets, value);
    }

    public long SentPackets
    {
        get => Interlocked.Read(ref _sentPackets);
        set => Interlocked.Exchange(ref _sentPackets, value);
    }

    public long ReceivedBytes
    {
        get => Interlocked.Read(ref _receivedBytes);
        set => Interlocked.Exchange(ref _receivedBytes, value);
    }

    public long SentBytes
    {
        get => Interlocked.Read(ref _sentBytes);
        set => Interlocked.Exchange(ref _sentBytes, value);
    }

    public void IncrementReceivedPackets()
    {
        Interlocked.Increment(ref _receivedPackets);
    }

    public void IncrementSentPackets()
    {
        Interlocked.Increment(ref _sentPackets);
    }

    public void AddReceivedBytes(long bytes)
    {
        Interlocked.Add(ref _receivedBytes, bytes);
    }

    public void AddSentBytes(long bytes)
    {
        Interlocked.Add(ref _sentBytes, bytes);
    }

    public void Reset()
    {
        Interlocked.Exchange(ref _receivedPackets, 0);
        Interlocked.Exchange(ref _sentPackets, 0);
        Interlocked.Exchange(ref _receivedBytes, 0);
        Interlocked.Exchange(ref _sentBytes, 0);
    }
}
