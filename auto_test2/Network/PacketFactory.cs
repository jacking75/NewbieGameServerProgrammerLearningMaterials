using CSCommon;

using MemoryPack;

namespace AutoTestClient.Network
{
    public class PacketFactory
    {
        public static byte[] GetReqLogin(string userID, string authToken = "TEST_TOKEN")
        {
            var packetData = new PKTReqLogin()
            {
                UserID = userID,
                AuthToken = authToken,
            };

            byte[] packet = MemoryPackSerializer.Serialize(packetData);

            PacketHeadReadWrite.Write((ushort)PacketID.ReqLogin, packet);

            return packet;
        }

        public static byte[] GetReqRoomEnter(Int32 roomNumber)
        {
            var packetData = new PKTReqRoomEnter()
            {
                RoomNumber = roomNumber
            };

            byte[] packet = MemoryPackSerializer.Serialize(packetData);

            PacketHeadReadWrite.Write((ushort)PacketID.ReqRoomEnter, packet);

            return packet;
        }

        public static byte[] GetReqRoomLeave()
        {
            var packetData = new PKTReqRoomLeave();

            var packet = MemoryPackSerializer.Serialize(packetData);

            PacketHeadReadWrite.Write((ushort)PacketID.ReqRoomLeave, packet);

            return packet;
        }

        public static byte[] GetReqRoomChat(string chatMessage)
        {
            var packetData = new PKTReqRoomChat()
            {
                ChatMessage = chatMessage
            };

            var packet = MemoryPackSerializer.Serialize(packetData);

            PacketHeadReadWrite.Write((ushort)PacketID.ReqRoomChat, packet);

            return packet;
        }

        public static byte[] GetReqGameStart()
        {
            var packetData = new PKTReqGameStart();

            var packet = MemoryPackSerializer.Serialize(packetData);

            PacketHeadReadWrite.Write((ushort)PacketID.ReqGameStart, packet);

            return packet;
        }

        public static byte[] GetReqPutStone(Int32 x, Int32 y)
        {
            var packetData = new PKTReqPutStone()
            {
                X = x,
                Y = y
            };

            var packet = MemoryPackSerializer.Serialize(packetData);

            PacketHeadReadWrite.Write((ushort)PacketID.ReqPutStone, packet);

            return packet;
        }
    }
}