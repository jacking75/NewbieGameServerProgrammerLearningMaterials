using OmokEngine.Core;
using OmokEngine.Evaluation;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Analysis;

/// <summary>
/// 렌주 규칙 체커 (금수 규칙)
/// </summary>
public class RenjuRuleChecker
{
    private OmokBoard board;

    public enum ForbiddenType
    {
        None,
        DoubleThree,   // 3-3 금수
        DoubleFour,    // 4-4 금수
        Overline,      // 장목 (6목 이상)
        Multiple       // 복합 금수
    }

    public class ForbiddenMoveInfo
    {
        public ForbiddenType Type { get; set; }
        public List<string> Reasons { get; set; }
        public List<Position> RelatedPositions { get; set; }
        public bool IsAllowed { get; set; }

        public ForbiddenMoveInfo()
        {
            Reasons = new List<string>();
            RelatedPositions = new List<Position>();
        }
    }

    public RenjuRuleChecker(OmokBoard board)
    {
        this.board = board;
    }

    /// <summary>
    /// 금수 체크 (흑만 적용)
    /// </summary>
    public ForbiddenMoveInfo CheckForbiddenMove(Position pos, Stone stone)
    {
        var info = new ForbiddenMoveInfo
        {
            Type = ForbiddenType.None,
            IsAllowed = true
        };

        // 백은 금수가 없음
        if (stone != Stone.Black)
            return info;

        // 임시로 돌 놓기
        board.PlaceStone(pos, stone);

        // 1. 즉시 승리하는 수는 금수가 아님
        if (board.CheckWin(pos, stone))
        {
            board.RemoveStone(pos);
            return info;
        }

        var forbiddenTypes = new List<ForbiddenType>();

        // 2. 장목 체크 (6목 이상)
        if (CheckOverline(pos, stone, info))
        {
            forbiddenTypes.Add(ForbiddenType.Overline);
        }

        // 3. 쌍사 체크 (4-4)
        if (CheckDoubleFour(pos, stone, info))
        {
            forbiddenTypes.Add(ForbiddenType.DoubleFour);
        }

        // 4. 쌍삼 체크 (3-3)
        if (CheckDoubleThree(pos, stone, info))
        {
            forbiddenTypes.Add(ForbiddenType.DoubleThree);
        }

        board.RemoveStone(pos);

        // 결과 설정
        if (forbiddenTypes.Count > 0)
        {
            info.IsAllowed = false;
            info.Type = forbiddenTypes.Count > 1
                ? ForbiddenType.Multiple
                : forbiddenTypes[0];
        }

        return info;
    }

    private bool CheckOverline(Position pos, Stone stone, ForbiddenMoveInfo info)
    {
        var directions = new[] { (0, 1), (1, 0), (1, 1), (1, -1) };

        foreach (var (dx, dy) in directions)
        {
            int count = 1;

            count += board.CountConsecutive(pos, stone, dx, dy);
            count += board.CountConsecutive(pos, stone, -dx, -dy);

            if (count > 5)
            {
                info.Reasons.Add($"장목: {count}개 연속 (6목 이상 금지)");
                return true;
            }
        }

        return false;
    }

    private bool CheckDoubleFour(Position pos, Stone stone, ForbiddenMoveInfo info)
    {
        int fourCount = 0;
        var directions = new[] { (0, 1), (1, 0), (1, 1), (1, -1) };

        foreach (var (dx, dy) in directions)
        {
            if (IsFourInDirection(pos, stone, dx, dy))
                fourCount++;
        }

        if (fourCount >= 2)
        {
            info.Reasons.Add($"쌍사: {fourCount}개의 4목 동시 생성");
            return true;
        }

        return false;
    }

    private bool CheckDoubleThree(Position pos, Stone stone, ForbiddenMoveInfo info)
    {
        int threeCount = 0;
        var directions = new[] { (0, 1), (1, 0), (1, 1), (1, -1) };

        foreach (var (dx, dy) in directions)
        {
            if (IsOpenThreeInDirection(pos, stone, dx, dy))
                threeCount++;
        }

        if (threeCount >= 2)
        {
            info.Reasons.Add($"쌍삼: {threeCount}개의 열린 3목 동시 생성");
            return true;
        }

        return false;
    }

    private bool IsFourInDirection(Position pos, Stone stone, int dx, int dy)
    {
        int count = 1;
        count += board.CountConsecutive(pos, stone, dx, dy);
        count += board.CountConsecutive(pos, stone, -dx, -dy);

        return count == 4;
    }

    private bool IsOpenThreeInDirection(Position pos, Stone stone, int dx, int dy)
    {
        var patterns = PatternAnalyzer.AnalyzePosition(board, pos, stone);

        string dirName = GetDirectionName(dx, dy);
        if (patterns.TryGetValue(dirName, out var pattern))
        {
            return pattern.ConsecutiveStones == 3 && pattern.OpenEnds == 2;
        }

        return false;
    }

    private string GetDirectionName(int dx, int dy)
    {
        if (dx == 0 && dy == 1) return "Horizontal";
        if (dx == 1 && dy == 0) return "Vertical";
        if (dx == 1 && dy == 1) return "Diagonal1";
        if (dx == 1 && dy == -1) return "Diagonal2";
        return "";
    }

    /// <summary>
    /// 모든 합법적인 수 가져오기
    /// </summary>
    public List<Position> GetLegalMoves(Stone stone)
    {
        var legalMoves = new List<Position>();
        int size = board.GetBoardSize();

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                var pos = new Position(i, j);
                if (board.IsEmpty(i, j))
                {
                    var info = CheckForbiddenMove(pos, stone);
                    if (info.IsAllowed)
                    {
                        legalMoves.Add(pos);
                    }
                }
            }
        }

        return legalMoves;
    }
}