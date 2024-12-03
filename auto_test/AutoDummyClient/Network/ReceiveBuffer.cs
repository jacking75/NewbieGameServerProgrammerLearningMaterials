using CSCommon;

namespace AutoTestClient.Network
{
    public class ReceiveBuffer
    {
        private byte[] _current;
        private byte[] _remain;
        private int _currentSize = 0;
        private int _remainSize = 0;

        private int _headerSize = 0;
        private int _packetMaxSize;

        public void Init(int capacity, int headerSize, int packetMaxSize = 1024)
        {
            _headerSize = headerSize;
            _packetMaxSize = packetMaxSize;
            _current = new byte[capacity * 2];
        }

        public byte[] Get() => _current;

        public void SetCurrentSize(int size) => _currentSize = size;

        public List<ReceivePacketInfo> GetCompletedPacketList()
        {
            var readPos = 0;
            var results = new List<ReceivePacketInfo>();

            while (true)
            {
                var currentSize = Combine();

                // 1. [예외] 현재 수신 버퍼의 데이터 사이즈가 프로토콜 헤더 크기보다 작은가?
                if (currentSize < _headerSize)
                {
                    break;
                }

                // 2. 프로토콜 헤더의 패킷 사이즈가 허용 가능 패킷 사이즈를 초과했는가?
                var packetSize = PacketHeadReadWrite.GetTotalSize(_current, readPos);
                if (packetSize > _packetMaxSize)
                {
                    // 이상한 유저다. 바로 끊어야한다.
                    return null;
                }

                // 3. 완성된 패킷을 만들기에는 현재 수신 버퍼의 데이터가 부족하다.
                if (packetSize > currentSize)
                {
                    break;
                }

                // 4. 패킷 생성.
                var packetInfo = new ReceivePacketInfo()
                {
                    Packet = new byte[packetSize]
                };
                Buffer.BlockCopy(_current, readPos, packetInfo.Packet, 0, packetSize);
                results.Add(packetInfo);

                // 5. 읽은 부분 제외
                _currentSize -= packetSize;

                // 6. 읽기 위치 변경
                readPos += packetSize;
            }

            // 7. 다음 처리를 위해 남은 바이트를 저장해둔다.
            Save(readPos);

            return results;
        }

        private int Combine()
        {
            if (_remainSize > 0)    // 버퍼에 찌꺼기가 존재하는가?
            {
                var moveOffset = _remainSize;

                // 기존 데이터를 남아있던 데이터의 길이만큼 앞으로 이동시킨다.
                Buffer.BlockCopy(_current, 0, _current, moveOffset, _currentSize);

                // 남아있던 데이터를 CurrentBuffer 맨 앞에 붙여넣는다.
                Buffer.BlockCopy(_remain, 0, _current, 0, _remainSize);

                _currentSize = _currentSize + _remainSize;

                _remainSize = 0;
            }

            return _currentSize;
        }

        private void Save(int srcOffset)
        {
            if (_currentSize != 0)
            {
                _remainSize = _currentSize;
                _remain = new byte[_remainSize];

                // 읽은 부분은 제외하고 남아있는 바이트를 remainBuffer로 복사.
                Buffer.BlockCopy(_current, srcOffset, _remain, 0, _remainSize);
            }
        }
    }
}