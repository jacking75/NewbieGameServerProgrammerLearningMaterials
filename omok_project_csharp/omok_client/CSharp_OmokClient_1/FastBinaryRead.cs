using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{
    public class FastBinaryRead
    {
        public static bool Boolean(byte[] bytes, int offset)
        {
            return (bytes[offset] == 0) ? false : true;
        }
        public static bool Boolean(ReadOnlySpan<byte> bytes, int offset)
        {
            return (bytes[offset] == 0) ? false : true;
        }

        public static byte Byte(byte[] bytes, int offset)
        {
            return bytes[offset];
        }
        public static byte Byte(ReadOnlySpan<byte> bytes, int offset)
        {
            return bytes[offset];
        }

        public static byte[] Bytes(byte[] bytes, int offset, int count)
        {
            var dest = new byte[count];
            Buffer.BlockCopy(bytes, offset, dest, 0, count);
            return dest;
        }

        public static sbyte SByte(byte[] bytes, int offset)
        {
            return (sbyte)bytes[offset];
        }
        public static sbyte SByte(ReadOnlySpan<byte> bytes, int offset)
        {
            return (sbyte)bytes[offset];
        }

        public static unsafe float Single(byte[] bytes, int offset)
        {
            if (offset % 4 == 0)
            {
                fixed (byte* ptr = bytes)
                {
                    return *(float*)(ptr + offset);
                }
            }
            else
            {
                uint num = (uint)((int)bytes[offset] | (int)bytes[offset + 1] << 8 | (int)bytes[offset + 2] << 16 | (int)bytes[offset + 3] << 24);
                return *(float*)(&num);
            }
        }
        public static unsafe float Single(ReadOnlySpan<byte> bytes, int offset)
        {
            if (offset % 4 == 0)
            {
                fixed (byte* ptr = bytes)
                {
                    return *(float*)(ptr + offset);
                }
            }
            else
            {
                uint num = (uint)((int)bytes[offset] | (int)bytes[offset + 1] << 8 | (int)bytes[offset + 2] << 16 | (int)bytes[offset + 3] << 24);
                return *(float*)(&num);
            }
        }

        public static unsafe double Double(byte[] bytes, int offset)
        {
            if (offset % 8 == 0)
            {
                fixed (byte* ptr = bytes)
                {
                    return *(double*)(ptr + offset);
                }
            }
            else
            {
                uint num = (uint)((int)bytes[offset] | (int)bytes[offset + 1] << 8 | (int)bytes[offset + 2] << 16 | (int)bytes[offset + 3] << 24);
                ulong num2 = (ulong)((int)bytes[offset + 4] | (int)bytes[offset + 5] << 8 | (int)bytes[offset + 6] << 16 | (int)bytes[offset + 7] << 24) << 32 | (ulong)num;
                return *(double*)(&num2);
            }
        }
        public static unsafe double Double(ReadOnlySpan<byte> bytes, int offset)
        {
            if (offset % 8 == 0)
            {
                fixed (byte* ptr = bytes)
                {
                    return *(double*)(ptr + offset);
                }
            }
            else
            {
                uint num = (uint)((int)bytes[offset] | (int)bytes[offset + 1] << 8 | (int)bytes[offset + 2] << 16 | (int)bytes[offset + 3] << 24);
                ulong num2 = (ulong)((int)bytes[offset + 4] | (int)bytes[offset + 5] << 8 | (int)bytes[offset + 6] << 16 | (int)bytes[offset + 7] << 24) << 32 | (ulong)num;
                return *(double*)(&num2);
            }
        }

        public static unsafe short Int16(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(short*)(ptr + offset);
            }
        }
        public static unsafe short Int16(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(short*)(ptr + offset);
            }
        }

        public static unsafe int Int32(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(int*)(ptr + offset);
            }
        }
        public static unsafe int Int32(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(int*)(ptr + offset);
            }
        }

        public static unsafe long Int64(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(long*)(ptr + offset);
            }
        }
        public static unsafe long Int64(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(long*)(ptr + offset);
            }
        }

        public static unsafe ushort UInt16(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(ushort*)(ptr + offset);
            }
        }
        public static unsafe ushort UInt16(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(ushort*)(ptr + offset);
            }
        }

        public static unsafe uint UInt32(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(uint*)(ptr + offset);
            }
        }
        public static unsafe uint UInt32(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(uint*)(ptr + offset);
            }
        }

        public static unsafe ulong UInt64(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(ulong*)(ptr + offset);
            }
        }
        public static unsafe ulong UInt64(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(ulong*)(ptr + offset);
            }
        }

        public static char Char(byte[] bytes, int offset)
        {
            return (char)FastBinaryRead.UInt16(bytes, offset);
        }
        public static char Char(ReadOnlySpan<byte> bytes, int offset)
        {
            return (char)FastBinaryRead.UInt16(bytes, offset);
        }

        public static string String(byte[] bytes, int offset, int count)
        {
            return StringEncoding.UTF8.GetString(bytes, offset, count);
        }

        public static unsafe decimal Decimal(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(Decimal*)(ptr + offset);
            }
        }
        public static unsafe decimal Decimal(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(Decimal*)(ptr + offset);
            }
        }

        public static unsafe Guid Guid(byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(Guid*)(ptr + offset);
            }
        }
        public static unsafe Guid Guid(ReadOnlySpan<byte> bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                return *(Guid*)(ptr + offset);
            }
        }

        #region Timestamp/Duration      
        public static unsafe TimeSpan TimeSpan(ref byte[] bytes, int offset)
        {
            checked
            {
                fixed (byte* ptr = bytes)
                {
                    var seconds = *(long*)(ptr + offset);
                    var nanos = *(int*)(ptr + offset + 8);

                    if (!Duration.IsNormalized(seconds, nanos))
                    {
                        throw new InvalidOperationException("Duration was not a valid normalized duration");
                    }
                    long ticks = seconds * System.TimeSpan.TicksPerSecond + nanos / Duration.NanosecondsPerTick;
                    return System.TimeSpan.FromTicks(ticks);
                }
            }
        }

        public static unsafe DateTime DateTime(ref byte[] bytes, int offset)
        {
            fixed (byte* ptr = bytes)
            {
                var seconds = *(long*)(ptr + offset);
                var nanos = *(int*)(ptr + offset + 8);

                if (!Timestamp.IsNormalized(seconds, nanos))
                {
                    throw new InvalidOperationException(string.Format(@"Timestamp contains invalid values: Seconds={0}; Nanos={1}", seconds, nanos));
                }
                return Timestamp.UnixEpoch.AddSeconds(seconds).AddTicks(nanos / Duration.NanosecondsPerTick);
            }
        }

        internal static class Timestamp
        {
            internal static readonly DateTime UnixEpoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            internal const long BclSecondsAtUnixEpoch = 62135596800;
            internal const long UnixSecondsAtBclMaxValue = 253402300799;
            internal const long UnixSecondsAtBclMinValue = -BclSecondsAtUnixEpoch;
            internal const int MaxNanos = Duration.NanosecondsPerSecond - 1;

            internal static bool IsNormalized(long seconds, int nanoseconds)
            {
                return nanoseconds >= 0 &&
                    nanoseconds <= MaxNanos &&
                    seconds >= UnixSecondsAtBclMinValue &&
                    seconds <= UnixSecondsAtBclMaxValue;
            }
        }

        internal static class Duration
        {
            public const int NanosecondsPerSecond = 1000000000;
            public const int NanosecondsPerTick = 100;
            public const long MaxSeconds = 315576000000L;
            public const long MinSeconds = -315576000000L;
            internal const int MaxNanoseconds = NanosecondsPerSecond - 1;
            internal const int MinNanoseconds = -NanosecondsPerSecond + 1;

            internal static bool IsNormalized(long seconds, int nanoseconds)
            {
                // Simple boundaries
                if (seconds < MinSeconds || seconds > MaxSeconds ||
                    nanoseconds < MinNanoseconds || nanoseconds > MaxNanoseconds)
                {
                    return false;
                }
                // We only have a problem is one is strictly negative and the other is
                // strictly positive.
                return Math.Sign(seconds) * Math.Sign(nanoseconds) != -1;
            }
        }
        #endregion
    }

    internal static class StringEncoding
    {
        public static Encoding UTF8 = new UTF8Encoding(false);
    }
}
