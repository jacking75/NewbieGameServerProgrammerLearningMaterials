using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Core;

/// <summary>
/// 오목 보드 클래스
/// </summary>
public class OmokBoard
{
    private const int BOARD_SIZE = 15;
    private Stone[,] board;
    private List<Position> moveHistory;

    // 4개 주요 방향 (가로, 세로, 대각선2개)
    private static readonly (int dx, int dy)[] Directions =
    {
            (0, 1),   // 가로
            (1, 0),   // 세로
            (1, 1),   // 대각선 \
            (1, -1)   // 대각선 /
        };

    public OmokBoard()
    {
        board = new Stone[BOARD_SIZE, BOARD_SIZE];
        moveHistory = new List<Position>();
        InitializeBoard();
    }

    private void InitializeBoard()
    {
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                board[i, j] = Stone.Empty;
    }

    public bool IsValidPosition(int row, int col)
    {
        return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
    }

    public bool IsEmpty(int row, int col)
    {
        return IsValidPosition(row, col) && board[row, col] == Stone.Empty;
    }

    public Stone GetStone(int row, int col)
    {
        return IsValidPosition(row, col) ? board[row, col] : Stone.Empty;
    }

    public bool PlaceStone(Position pos, Stone stone)
    {
        if (!IsEmpty(pos.Row, pos.Col) || stone == Stone.Empty)
            return false;

        board[pos.Row, pos.Col] = stone;
        moveHistory.Add(pos);
        return true;
    }

    public bool RemoveStone(Position pos)
    {
        if (!IsValidPosition(pos.Row, pos.Col))
            return false;

        board[pos.Row, pos.Col] = Stone.Empty;
        if (moveHistory.Count > 0 && moveHistory[moveHistory.Count - 1].Equals(pos))
            moveHistory.RemoveAt(moveHistory.Count - 1);
        return true;
    }

    public List<Position> GetEmptyPositions()
    {
        var positions = new List<Position>();
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                if (board[i, j] == Stone.Empty)
                    positions.Add(new Position(i, j));
        return positions;
    }

    /// <summary>
    /// 특정 위치 주변에 돌이 있는지 확인 (탐색 공간 축소용)
    /// </summary>
    public bool HasNeighbor(Position pos, int distance = 2)
    {
        for (int i = Math.Max(0, pos.Row - distance);
             i <= Math.Min(BOARD_SIZE - 1, pos.Row + distance); i++)
        {
            for (int j = Math.Max(0, pos.Col - distance);
                 j <= Math.Min(BOARD_SIZE - 1, pos.Col + distance); j++)
            {
                if (board[i, j] != Stone.Empty)
                    return true;
            }
        }
        return false;
    }

    /// <summary>
    /// 승리 체크
    /// </summary>
    public bool CheckWin(Position lastMove, Stone stone)
    {
        if (stone == Stone.Empty) return false;

        foreach (var (dx, dy) in Directions)
        {
            int count = 1; // 현재 돌 포함

            // 양방향 체크
            count += CountConsecutive(lastMove, stone, dx, dy);
            count += CountConsecutive(lastMove, stone, -dx, -dy);

            if (count >= 5)
                return true;
        }

        return false;
    }

    /// <summary>
    /// 특정 방향으로 연속된 돌의 개수 세기
    /// </summary>
    public int CountConsecutive(Position pos, Stone stone, int dx, int dy)
    {
        int count = 0;
        int row = pos.Row + dx;
        int col = pos.Col + dy;

        while (IsValidPosition(row, col) && board[row, col] == stone)
        {
            count++;
            row += dx;
            col += dy;
        }

        return count;
    }

    public Stone[,] GetBoardCopy()
    {
        return (Stone[,])board.Clone();
    }

    public int GetBoardSize() => BOARD_SIZE;

    public List<Position> GetMoveHistory() => new List<Position>(moveHistory);

    public void Clear()
    {
        InitializeBoard();
        moveHistory.Clear();
    }
}