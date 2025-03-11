using System;

using MemoryPack;

namespace CSCommon
{
    public struct PacketHeadReadWrite
    {
        const Int32 StartPos = 1;
        const Int32 PacketIDPos = 3;
        public const Int32 HeadSize = 6;

        public UInt16 TotalSize;
        public UInt16 ID;
        public Byte Type;

        public static UInt16 GetTotalSize(Byte[] data, Int32 startPos)
        {
            return FastBinaryRead.UInt16(data, startPos + StartPos);
        }

        public static UInt16 ReadPacketID(Byte[] data, Int32 startPos = 0)
        {
            return FastBinaryRead.UInt16(data, startPos + PacketIDPos);
        }

        public void Read(Byte[] headerData)
        {
            var pos = StartPos;

            TotalSize = FastBinaryRead.UInt16(headerData, pos);
            pos += 2;

            ID = FastBinaryRead.UInt16(headerData, pos);
            pos += 2;

            Type = headerData[pos];
            pos += 1;
        }

        public void Write(Byte[] pktData)
        {
            var pos = StartPos;

            FastBinaryWrite.UInt16(pktData, pos, TotalSize);
            pos += 2;

            FastBinaryWrite.UInt16(pktData, pos, ID);
            pos += 2;

            pktData[pos] = Type;
            pos += 1;
        }

        public static void Write(UInt16 packetID, Byte[] pktData)
        {
            var pos = StartPos;

            FastBinaryWrite.UInt16(pktData, pos, (UInt16)pktData.Length);
            pos += 2;

            FastBinaryWrite.UInt16(pktData, pos, packetID);
            pos += 2;

            pktData[pos] = 0;
            pos += 1;
        }
    }

    [MemoryPackable]
    public partial class PacketHead
    {
        public UInt16 TotalSize { get; set; } = 0;
        public UInt16 ID { get; set; } = 0;
        public Byte Type { get; set; } = 0;
    }
}