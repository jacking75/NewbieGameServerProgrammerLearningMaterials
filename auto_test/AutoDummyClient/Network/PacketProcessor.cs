using System.Threading.Tasks.Dataflow;

using AutoTestClient.Dummy;
using AutoTestClient.Network.S2CPacketHandler;

using CSCommon;

namespace AutoTestClient.Network
{
    public class PacketProcessor
    {
        public Func<Int32, DummyObject> GetDummyByIndexFunc;

        public BufferBlock<ReceivePacketInfo> Packets { get; private set; } = new();

        private bool _isRunning;
        private Thread _updater;
        private Dictionary<ushort, BaseHandler> _handlers = new();

        public void Init()
        {
            _handlers[(ushort)PacketID.ResLogin] = new ResponseLogin();
            _handlers[(ushort)PacketID.ResRoomEnter] = new ResponseRoomEnter();
            _handlers[(ushort)PacketID.ResRoomLeave] = new ResponseRoomLeave();
            _handlers[(ushort)PacketID.ResGameStart] = new ResponseGameStart();
            _handlers[(ushort)PacketID.ResPutStone] = new ResponsePutStone();

            _handlers[(ushort)PacketID.NtfRoomUserList] = new NotifyRoomUserList();
            _handlers[(ushort)PacketID.NtfRoomNewUser] = new NotifyRoomNewUser();
            _handlers[(ushort)PacketID.NtfRoomLeaveUser] = new NotifyRoomLeaveUser();
            _handlers[(ushort)PacketID.NtfRoomChat] = new NotifyRoomChat();
            _handlers[(ushort)PacketID.NtfGameStart] = new NotifyGameStart();
            _handlers[(ushort)PacketID.NtfPutStone] = new NotifyPutStone();

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
            Packets.Complete();
            _updater.Join();
        }

        public Int32 GetPacketCount() => Packets.Count;

        public void Add(ReceivePacketInfo packetInfo)
        {
            Packets.Post(packetInfo);
        }

        private void Work()
        {
            while (_isRunning == true)
            {
                try
                {
                    ReceivePacketInfo packetInfo = Packets.Receive();
                    var (dummy, packetID) = Middleware(packetInfo);

                    if (dummy is not null)
                    {
                        _handlers[packetID].Handle(dummy, packetInfo.Packet);
                    }
                }
                catch (Exception ex)
                {
                    // Stop 함수에 의해 들어온거는 예외가 아니다.
                    if (_isRunning == false)
                    {
                        return;
                    }

                    Console.WriteLine(ex.ToString());
                }
            }
        }

        private (DummyObject, ushort) Middleware(ReceivePacketInfo packetInfo)
        {
            var dummy = GetDummyByIndexFunc(packetInfo.DummyIndex);
            if (dummy is null)
            {
                return (null, 0);
            }

            // 패킷 아이디 확인
            var packetID = PacketHeadReadWrite.ReadPacketID(packetInfo.Packet);

            // 응답 받아야하는 패킷은 RTT 계산과 모니터링 정보를 수정한다.
            if (packetID == (ushort)PacketID.ResLogin
                || packetID == (ushort)PacketID.ResRoomEnter
                || packetID == (ushort)PacketID.ResRoomLeave
                || packetID == (ushort)PacketID.ResGameStart
                || packetID == (ushort)PacketID.ResPutStone)
            {
                Monitor.DecreaseWaitForResponsePacketCount();
                dummy.UpdateRTT();
            }

            return (dummy, packetID);
        }
    }
}