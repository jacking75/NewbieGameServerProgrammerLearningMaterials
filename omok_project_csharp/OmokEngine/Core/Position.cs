using System;

namespace OmokEngine.Core;

/// <summary>
/// 보드 위의 좌표
/// </summary>
public readonly struct Position : IEquatable<Position>
{
    public int Row { get; }
    public int Col { get; }

    public Position(int row, int col)
    {
        Row = row;
        Col = col;
    }

    public bool Equals(Position other)
    {
        return Row == other.Row && Col == other.Col;
    }

    public override bool Equals(object? obj)
    {
        return obj is Position other && Equals(other);
    }

    public override int GetHashCode()
    {
        return HashCode.Combine(Row, Col);
    }

    public static bool operator ==(Position left, Position right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(Position left, Position right)
    {
        return !left.Equals(right);
    }

    public override string ToString()
    {
        return $"({Row}, {Col})";
    }
}


/// <summary>
/// 평가된 수
/// </summary>
public class EvaluatedMove
{
    public Position Position { get; set; }
    public int Score { get; set; }
    public MoveType Type { get; set; }
    public string Description { get; set; }

    public EvaluatedMove(Position pos, int score, MoveType type, string desc = "")
    {
        Position = pos;
        Score = score;
        Type = type;
        Description = desc;
    }

    public override string ToString()
    {
        return $"Move: {Position}, Score: {Score}, Type: {Type}";
    }
}