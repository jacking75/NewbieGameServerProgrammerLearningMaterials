using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{
    public class FastBinaryWrite
    {
        public static void Boolean(byte[] bytes, int offset, bool value)
        {
            bytes[offset] = (byte)(value ? 1 : 0);
        }

        public static void BooleanTrueUnsafe(byte[] bytes, int offset)
        {
            bytes[offset] = (byte)(1);
        }

        public static void BooleanFalseUnsafe(byte[] bytes, int offset)
        {
            bytes[offset] = (byte)(0);
        }

        public static void Byte(byte[] bytes, int offset, byte value)
        {
            bytes[offset] = value;
        }

        public static int Bytes(byte[] bytes, int offset, byte[] value)
        {
            Buffer.BlockCopy(value, 0, bytes, offset, value.Length);
            return value.Length;
        }

        public static int SByte(byte[] bytes, int offset, sbyte value)
        {
            bytes[offset] = (byte)value;
            return 1;
        }

        public static unsafe int Single(byte[] bytes, int offset, float value)
        {
            if (offset % 4 == 0)
            {
                fixed (byte* ptr = bytes)
                {
                    *(float*)(ptr + offset) = value;
                }
            }
            else
            {
                uint num = *(uint*)(&value);
                bytes[offset] = (byte)num;
                bytes[offset + 1] = (byte)(num >> 8);
                bytes[offset + 2] = (byte)(num >> 16);
                bytes[offset + 3] = (byte)(num >> 24);
            }

            return 4;
        }

        public static unsafe int Double(byte[] bytes, int offset, double value)
        {
            if (offset % 8 == 0)
            {
                fixed (byte* ptr = bytes)
                {
                    *(double*)(ptr + offset) = value;
                }
            }
            else
            {
                ulong num = (ulong)(*(long*)(&value));
                bytes[offset] = (byte)num;
                bytes[offset + 1] = (byte)(num >> 8);
                bytes[offset + 2] = (byte)(num >> 16);
                bytes[offset + 3] = (byte)(num >> 24);
                bytes[offset + 4] = (byte)(num >> 32);
                bytes[offset + 5] = (byte)(num >> 40);
                bytes[offset + 6] = (byte)(num >> 48);
                bytes[offset + 7] = (byte)(num >> 56);
            }

            return 8;
        }

        public static unsafe int Int16(byte[] bytes, int offset, short value)
        {
            fixed (byte* ptr = bytes)
            {
                *(short*)(ptr + offset) = value;
            }

            return 2;
        }

        public static unsafe int Int32(byte[] bytes, int offset, int value)
        {
            fixed (byte* ptr = bytes)
            {
                *(int*)(ptr + offset) = value;
            }

            return 4;
        }

        public static unsafe void Int32Unsafe(byte[] bytes, int offset, int value)
        {
            fixed (byte* ptr = bytes)
            {
                *(int*)(ptr + offset) = value;
            }
        }

        public static unsafe int Int64(byte[] bytes, int offset, long value)
        {
            fixed (byte* ptr = bytes)
            {
                *(long*)(ptr + offset) = value;
            }

            return 8;
        }

        public static unsafe int UInt16(byte[] bytes, int offset, ushort value)
        {
            fixed (byte* ptr = bytes)
            {
                *(ushort*)(ptr + offset) = value;
            }

            return 2;
        }

        public static unsafe int UInt32(byte[] bytes, int offset, uint value)
        {
            fixed (byte* ptr = bytes)
            {
                *(uint*)(ptr + offset) = value;
            }

            return 4;
        }

        public static unsafe int UInt64(byte[] bytes, int offset, ulong value)
        {
            fixed (byte* ptr = bytes)
            {
                *(ulong*)(ptr + offset) = value;
            }

            return 8;
        }

        public static int Char(byte[] bytes, int offset, char value)
        {
            return UInt16(bytes, offset, (ushort)value);
        }

        public static int String(byte[] bytes, int offset, string value)
        {
            return StringEncoding.UTF8.GetBytes(value, 0, value.Length, bytes, offset);
        }

        public static unsafe int Decimal(byte[] bytes, int offset, decimal value)
        {
            fixed (byte* ptr = bytes)
            {
                *(Decimal*)(ptr + offset) = value;
            }

            return 16;
        }

        public static unsafe int Guid(byte[] bytes, int offset, Guid value)
        {
            fixed (byte* ptr = bytes)
            {
                *(Guid*)(ptr + offset) = value;
            }

            return 16;
        }

        #region Timestamp/Duration
        public static unsafe int TimeSpan(ref byte[] bytes, int offset, TimeSpan timeSpan)
        {
            checked
            {
                long ticks = timeSpan.Ticks;
                long seconds = ticks / System.TimeSpan.TicksPerSecond;
                int nanos = (int)(ticks % System.TimeSpan.TicksPerSecond) * Duration.NanosecondsPerTick;

                fixed (byte* ptr = bytes)
                {
                    *(long*)(ptr + offset) = seconds;
                    *(int*)(ptr + offset + 8) = nanos;
                }

                return 12;
            }
        }

        public static unsafe int DateTime(ref byte[] bytes, int offset, DateTime dateTime)
        {
            dateTime = dateTime.ToUniversalTime();

            // Do the arithmetic using DateTime.Ticks, which is always non-negative, making things simpler.
            long secondsSinceBclEpoch = dateTime.Ticks / System.TimeSpan.TicksPerSecond;
            int nanoseconds = (int)(dateTime.Ticks % System.TimeSpan.TicksPerSecond) * Duration.NanosecondsPerTick;

            fixed (byte* ptr = bytes)
            {
                *(long*)(ptr + offset) = (secondsSinceBclEpoch - Timestamp.BclSecondsAtUnixEpoch);
                *(int*)(ptr + offset + 8) = nanoseconds;
            }

            return 12;
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
}
