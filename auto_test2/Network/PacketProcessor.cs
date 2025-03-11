using System.Threading.Tasks.Dataflow;

using AutoTestClient.Dummy;
using AutoTestClient.PacketHandler;
using CSCommon;

namespace AutoTestClient.Network;

public class PacketProcessor
{
    DummyObject _dummy;
    //public Func<Int32, DummyObject> GetDummyByIndexFunc;

    public Queue<ReceivePacketInfo> Packets { get; private set; } = new();
                    
    private Dictionary<ushort, BaseHandler> _handlers = new();

    public void Init(DummyObject dummy)
    {
        _dummy = dummy;

        _handlers[(ushort)PacketID.ResLogin] = new ResponseLogin();
        _handlers[(ushort)PacketID.ResRoomEnter] = new ResponseRoomEnter();
        _handlers[(ushort)PacketID.ResRoomLeave] = new ResponseRoomLeave();
        
        _handlers[(ushort)PacketID.NtfRoomUserList] = new NotifyRoomUserList();
        _handlers[(ushort)PacketID.NtfRoomNewUser] = new NotifyRoomNewUser();
        _handlers[(ushort)PacketID.NtfRoomLeaveUser] = new NotifyRoomLeaveUser();
        _handlers[(ushort)PacketID.NtfRoomChat] = new NotifyRoomChat();        
    }

            
    public Int32 GetPacketCount() => Packets.Count;

    public void AddPacket(ReceivePacketInfo packetInfo)
    {
        Packets.Enqueue(packetInfo);
    }

    public void Update()
    {
        if (Packets.Count == 0)
        {
            return;
        }

        ReceivePacketInfo packetInfo = Packets.Dequeue();

        try
        {            
            var packetID = PacketHeadReadWrite.ReadPacketID(packetInfo.Packet);

            _handlers[packetID].Handle(_dummy, packetInfo.Packet);            
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex.ToString());
        }
    }


}