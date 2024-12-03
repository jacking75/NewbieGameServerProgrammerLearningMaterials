using AutoTestClient.Network;

namespace AutoTestClient.Dummy
{
    public class DummyNetwork : DummyBase
    {
        public static Action<ReceivePacketInfo> AddPacketToPacketProcessorFunc;

        public long RTT { get; private set; }

        public long ConnectedCount { get; private set; }

        private readonly Int32 _recvBufferSize = 1024;

        private readonly ReceiveBuffer _recvBuffer = new();

        private string _remoteIP;

        private Int32 _remotePort;

        private CustomSocket _connection;

        private DateTime _sendTime;

        public void UpdateRTT()
        {
            RTT = (DateTime.Now - _sendTime).Milliseconds;
        }

        public ErrorCode ReceiveAndAddPacketToPacketProcessor()
        {
            var (socketError, recvBytes) = _connection.Receive(_recvBuffer.Get());
            if (socketError != 0)
            {
                return ErrorCode.FailedReceivePacketException;
            }

            if (recvBytes == 0)
            {
                return ErrorCode.None;
            }

            _recvBuffer.SetCurrentSize(recvBytes);
            var packets = _recvBuffer.GetCompletedPacketList();

            if (packets is null)
            {
                return ErrorCode.FailedReceiveRangeOverPacketSize;
            }

            if (packets.Count >= 1)
            {
                AddPacketToPacketProcessor(packets);
            }

            return ErrorCode.None;
        }

        protected async Task<ErrorCode> Connect()
        {
            var socketError = await _connection.Connect(_remoteIP, _remotePort);
            if (socketError != 0)
            {
                if (socketError == 10061)   // 서버에 접속 자체가 불가능한 경우는 그냥 실패
                {
                    return ErrorCode.FailedConnect;
                }

                if (socketError == 10048)   // 서버쪽에서 아직 소켓 닫는 중이었다면, 잠시 대기 후 재시도.
                {
                    return ErrorCode.FailedConnect;
                }

                return ErrorCode.FailedConnect;
            }
            else
            {
                ++ConnectedCount;
                return ErrorCode.None;
            }
        }

        protected ErrorCode Disconnect()
        {
            var socketError = _connection.Close();
            if (socketError != 0)
            {
                return ErrorCode.FailedDisconnect;
            }

            return ErrorCode.None;
        }

        protected async Task<ErrorCode> SendPacket(byte[] source)
        {
            var (socketError, sendBytes) = await _connection.Send(source);

            _sendTime = DateTime.Now;

            if (socketError != 0)
            {
                return ErrorCode.FailedSend;
            }

            return ErrorCode.None;
        }

        protected void AddPacketToPacketProcessor(List<ReceivePacketInfo> packets)
        {
            foreach (var packet in packets)
            {
                packet.DummyIndex = Index;
                AddPacketToPacketProcessorFunc(packet);
            }
        }

        protected void InitNetworkConfig(ScenarioRunnerConfig config)
        {
            _connection = new CustomSocket();

            var remoteInfos = config.RemoteEndPoint.Value.Split(":");
            _remoteIP = remoteInfos[0];
            _remotePort = Int32.Parse(remoteInfos[1]);
            _recvBuffer.Init(_recvBufferSize, CSCommon.PacketHeadReadWrite.HeadSize);
        }
    }
}

