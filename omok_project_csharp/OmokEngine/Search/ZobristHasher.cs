using OmokEngine.Core;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Search;

/// <summary>
/// Zobrist 해싱 (빠르고 충돌이 적은 해시)
/// </summary>
public class ZobristHasher
{
    private ulong[,,] zobristTable = null!;  // [row, col, stone]
    private readonly Random random;
    private const int BOARD_SIZE = 15;

    public ZobristHasher(int seed = 12345)
    {
        random = new Random(seed);
        InitializeZobristTable();
    }

    private void InitializeZobristTable()
    {
        zobristTable = new ulong[BOARD_SIZE, BOARD_SIZE, 3];  // Empty, Black, White

        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    zobristTable[i, j, k] = GetRandomULong();
                }
            }
        }
    }

    /// <summary>
    /// 전체 보드 해시 계산
    /// </summary>
    public ulong ComputeHash(OmokBoard board)
    {
        ulong hash = 0;
        int size = board.GetBoardSize();

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                Stone stone = board.GetStone(i, j);
                if (stone != Stone.Empty)
                {
                    hash ^= zobristTable[i, j, (int)stone];
                }
            }
        }

        return hash;
    }

    /// <summary>
    /// 증분 해시 업데이트 (돌을 놓을 때)
    /// </summary>
    public ulong UpdateHash(ulong currentHash, Position pos, Stone stone)
    {
        return currentHash ^ zobristTable[pos.Row, pos.Col, (int)stone];
    }

    /// <summary>
    /// 증분 해시 업데이트 (돌을 제거할 때)
    /// </summary>
    public ulong RemoveFromHash(ulong currentHash, Position pos, Stone stone)
    {
        return currentHash ^ zobristTable[pos.Row, pos.Col, (int)stone];
    }

    private ulong GetRandomULong()
    {
        byte[] buffer = new byte[8];
        random.NextBytes(buffer);
        return BitConverter.ToUInt64(buffer, 0);
    }
}