using System.Net.Sockets;

namespace AutoTestClient.Network
{
    public class CustomSocket
    {
        private bool _isSelfDisconnected = false;
        private Socket _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

        public bool IsConnected()
        {
            return _socket != null && _socket.Connected;
        }

        public async Task<Int32> Connect(string ip, Int32 port)
        {
            try
            {
                // 재연결 시 서버 IP, Port를 재사용하기 위한 설정.
                _socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);

                // 빠른 테스트를 위해 TIME_WAIT 소켓을 생성하지 않는다.
                _socket.LingerState = new LingerOption(true, 0);

                await _socket.ConnectAsync(ip, port);

                // 네이글 알고리즘 OFF
                _socket.NoDelay = true;

                // 서버에서 끊은 걸 판단하기 위한 플래그.
                _isSelfDisconnected = false;

                return 0;
            }
            catch (Exception ex)
            {
                return ((SocketException)ex).ErrorCode;
            }
        }

        public Int32 Close()
        {
            try
            {
                // 더미에서 끊은건 에러로 판단하지 않기 위한 플래그.
                _isSelfDisconnected = true;

                _socket.Close();

                _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                return 0;
            }
            catch (Exception ex)
            {
                return ((SocketException)ex).ErrorCode;
            }
        }

        public (Int32 socketError, Int32 recvBytes) Receive(byte[] destination)
        {
            try
            {
                if (_socket.Poll(1000, SelectMode.SelectRead) == false)
                {
                    return (0, 0);
                }

                var recvBytes = _socket.Receive(destination);

                return (0, recvBytes);
            }
            catch (Exception ex)
            {
                // 수신 받는 스레드와 연결을 끊는 스레드가 서로 다른 경우, 수신 받는 스레드에서는 예외가 발생한다.
                // 다만, 더미에서 끊은 경우는 에러가 아니므로, 에러 코드를 반환하지 않는다.
                if (_isSelfDisconnected == true)
                {
                    return (0, 0);
                }

                return (((SocketException)ex).ErrorCode, 0);
            }
        }

        public async Task<(Int32 socketError, Int32 sendBytes)> Send(byte[] source)
        {
            try
            {
                var sendBytes = await _socket.SendAsync(source);
                return (0, sendBytes);
            }
            catch (Exception ex)
            {
                return (((SocketException)ex).ErrorCode, 0);
            }
        }
    }
}


