using OmokEngine.Core;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Evaluation;

/// <summary>
/// 패턴 분석 클래스
/// </summary>
public static class PatternAnalyzer
{
    // 패턴별 점수 정의
    private static readonly Dictionary<string, int> PatternScores = new Dictionary<string, int>
    {
        ["Five"] = 100000,           // 5목 (승리)
        ["OpenFour"] = 50000,        // 열린 4목 (다음 수 승리)
        ["Four"] = 10000,            // 4목
        ["OpenThree"] = 5000,        // 열린 3목
        ["Three"] = 1000,            // 3목
        ["OpenTwo"] = 500,           // 열린 2목
        ["Two"] = 100,               // 2목
        ["One"] = 10                 // 1목
    };

    /// <summary>
    /// 특정 위치의 모든 방향에서 패턴 분석
    /// </summary>
    public static Dictionary<string, Pattern> AnalyzePosition(OmokBoard board, Position pos, Stone stone)
    {
        var patterns = new Dictionary<string, Pattern>();
        var directions = new[] { (0, 1), (1, 0), (1, 1), (1, -1) };
        string[] dirNames = { "Horizontal", "Vertical", "Diagonal1", "Diagonal2" };

        for (int i = 0; i < directions.Length; i++)
        {
            var (dx, dy) = directions[i];
            var pattern = AnalyzeDirection(board, pos, stone, dx, dy);
            patterns[dirNames[i]] = pattern;
        }

        return patterns;
    }

    /// <summary>
    /// 한 방향으로 패턴 분석
    /// </summary>
    private static Pattern AnalyzeDirection(OmokBoard board, Position pos, Stone stone, int dx, int dy)
    {
        int consecutive = 1;  // 현재 위치 포함
        int openEnds = 0;
        int totalLength = 1;
        bool hasSpace = false;

        // 정방향 탐색
        var (forwardConsec, forwardOpen, forwardSpace, forwardLen) =
            ScanDirection(board, pos, stone, dx, dy);

        // 역방향 탐색
        var (backwardConsec, backwardOpen, backwardSpace, backwardLen) =
            ScanDirection(board, pos, stone, -dx, -dy);

        consecutive += forwardConsec + backwardConsec;
        openEnds = forwardOpen + backwardOpen;
        hasSpace = forwardSpace || backwardSpace;
        totalLength += forwardLen + backwardLen;

        return new Pattern(consecutive, openEnds, hasSpace, totalLength);
    }

    /// <summary>
    /// 한쪽 방향으로 스캔
    /// </summary>
    private static (int consecutive, int openEnd, bool hasSpace, int length)
        ScanDirection(OmokBoard board, Position pos, Stone stone, int dx, int dy)
    {
        int consecutive = 0;
        int length = 0;
        bool hasSpace = false;
        bool foundSpace = false;
        int openEnd = 0;

        int row = pos.Row + dx;
        int col = pos.Col + dy;

        // 최대 5칸까지만 탐색 (오목은 5개만 필요)
        for (int i = 0; i < 5; i++)
        {
            if (!board.IsValidPosition(row, col))
                break;

            Stone current = board.GetStone(row, col);

            if (current == stone)
            {
                consecutive++;
                length++;
            }
            else if (current == Stone.Empty && !foundSpace && consecutive > 0)
            {
                // 첫 번째 빈 공간 (띄어진 패턴)
                hasSpace = true;
                foundSpace = true;
                length++;
            }
            else if (current == Stone.Empty)
            {
                // 열린 끝
                openEnd = 1;
                break;
            }
            else
            {
                // 상대 돌로 막힘
                break;
            }

            row += dx;
            col += dy;
        }

        return (consecutive, openEnd, hasSpace, length);
    }

    /// <summary>
    /// 패턴에 기반한 점수 계산
    /// </summary>
    public static int CalculatePatternScore(Pattern pattern)
    {
        int consec = pattern.ConsecutiveStones;
        int open = pattern.OpenEnds;

        // 5목 이상
        if (consec >= 5)
            return PatternScores["Five"];

        // 4목
        if (consec == 4)
        {
            if (open == 2)
                return PatternScores["OpenFour"];  // 양쪽 열림 (필승)
            else if (open == 1)
                return PatternScores["Four"];      // 한쪽 막힘
            else
                return PatternScores["Four"] / 2;  // 양쪽 막힘
        }

        // 3목
        if (consec == 3)
        {
            if (open == 2)
                return PatternScores["OpenThree"];
            else if (open == 1)
                return PatternScores["Three"];
            else
                return PatternScores["Three"] / 2;
        }

        // 2목
        if (consec == 2)
        {
            if (open == 2)
                return PatternScores["OpenTwo"];
            else if (open == 1)
                return PatternScores["Two"];
            else
                return PatternScores["Two"] / 2;
        }

        // 1목
        if (consec == 1)
            return PatternScores["One"];

        return 0;
    }
}