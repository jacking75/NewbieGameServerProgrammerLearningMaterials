using OmokEngine.Core;
using OmokEngine.Evaluation;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.AI;

public enum PlayerSkillLevel
{
    Novice,        // 0-40점
    Beginner,      // 40-55점
    Intermediate,  // 55-70점
    Advanced,      // 70-85점
    Expert         // 85-100점
}

public class PlayerWeaknesses
{
    public bool WeakDefense { get; set; }
    public bool WeakAttack { get; set; }
    public bool InconsistentPlay { get; set; }
    public bool TooFast { get; set; }
    public bool TooSlow { get; set; }
}

/// <summary>
/// 플레이어 실력 분석기
/// </summary>
public class PlayerSkillAnalyzer
{
    public List<MoveQuality> moveHistory = new List<MoveQuality>();
    private int recentMovesWindow = 10;

    public class MoveQuality
    {
        public Position PlayerMove { get; set; }
        public int OptimalScore { get; set; }
        public int ActualScore { get; set; }
        public double Quality { get; set; }
        public bool MissedThreat { get; set; }
        public bool MissedOpportunity { get; set; }
        public long ThinkingTime { get; set; }
    }

    /// <summary>
    /// 플레이어가 둔 수의 품질 평가
    /// </summary>
    public MoveQuality AnalyzePlayerMove(OmokBoard board, Position playerMove,
                                        Stone playerStone, long thinkingTimeMs)
    {
        var evaluator = new MoveEvaluator(board);

        // 최선의 수들 가져오기
        var topMoves = evaluator.EvaluateAllMoves(playerStone, 10);
        var optimalMove = topMoves[0];

        // 플레이어가 둔 수의 점수 찾기
        var actualMove = topMoves.FirstOrDefault(m => m.Position.Equals(playerMove));
        int actualScore = actualMove?.Score ?? 0;

        // 품질 계산 (0.0 ~ 1.0)
        double quality = optimalMove.Score > 0
            ? (double)actualScore / optimalMove.Score
            : 1.0;

        // 위협/기회 놓침 분석
        bool missedThreat = CheckMissedThreat(topMoves, actualMove!);
        bool missedOpportunity = CheckMissedOpportunity(topMoves, actualMove!);

        var moveQuality = new MoveQuality
        {
            PlayerMove = playerMove,
            OptimalScore = optimalMove.Score,
            ActualScore = actualScore,
            Quality = quality,
            MissedThreat = missedThreat,
            MissedOpportunity = missedOpportunity,
            ThinkingTime = thinkingTimeMs
        };

        moveHistory.Add(moveQuality);

        return moveQuality;
    }

    private bool CheckMissedThreat(List<EvaluatedMove> topMoves, EvaluatedMove actualMove)
    {
        var defensiveMoves = topMoves.Take(3).Where(m =>
            m.Type == MoveType.DefendFour ||
            m.Type == MoveType.DefendThree);

        if (!defensiveMoves.Any())
            return false;

        return actualMove == null ||
               (actualMove.Type != MoveType.DefendFour &&
                actualMove.Type != MoveType.DefendThree);
    }

    private bool CheckMissedOpportunity(List<EvaluatedMove> topMoves, EvaluatedMove actualMove)
    {
        var opportunityMoves = topMoves.Take(3).Where(m =>
            m.Type == MoveType.Winning ||
            m.Type == MoveType.MakeFour);

        return opportunityMoves.Any() &&
               (actualMove == null ||
                (actualMove.Type != MoveType.Winning &&
                 actualMove.Type != MoveType.MakeFour));
    }

    /// <summary>
    /// 현재 실력 레벨 계산
    /// </summary>
    public PlayerSkillLevel GetCurrentSkillLevel()
    {
        if (moveHistory.Count < 5)
            return PlayerSkillLevel.Beginner;

        var recentMoves = moveHistory.TakeLast(recentMovesWindow).ToList();

        double avgQuality = recentMoves.Average(m => m.Quality);
        double threatMissRate = recentMoves.Count(m => m.MissedThreat) / (double)recentMoves.Count;
        double opportunityMissRate = recentMoves.Count(m => m.MissedOpportunity) / (double)recentMoves.Count;

        if (avgQuality > 0.85 && threatMissRate < 0.1 && opportunityMissRate < 0.15)
            return PlayerSkillLevel.Expert;
        else if (avgQuality > 0.70 && threatMissRate < 0.2 && opportunityMissRate < 0.3)
            return PlayerSkillLevel.Advanced;
        else if (avgQuality > 0.55 && threatMissRate < 0.35)
            return PlayerSkillLevel.Intermediate;
        else if (avgQuality > 0.40)
            return PlayerSkillLevel.Beginner;
        else
            return PlayerSkillLevel.Novice;
    }

    /// <summary>
    /// 상세한 실력 점수 (0-100)
    /// </summary>
    public double GetSkillScore()
    {
        if (moveHistory.Count < 3)
            return 50.0;

        var recentMoves = moveHistory.TakeLast(recentMovesWindow).ToList();

        double qualityScore = recentMoves.Average(m => m.Quality) * 60;
        double threatScore = (1 - recentMoves.Count(m => m.MissedThreat) / (double)recentMoves.Count) * 25;
        double opportunityScore = (1 - recentMoves.Count(m => m.MissedOpportunity) / (double)recentMoves.Count) * 15;

        return Math.Min(100, qualityScore + threatScore + opportunityScore);
    }

    /// <summary>
    /// 플레이어의 약점 분석
    /// </summary>
    public PlayerWeaknesses AnalyzeWeaknesses()
    {
        var recentMoves = moveHistory.TakeLast(20).ToList();

        if (recentMoves.Count == 0)
            return new PlayerWeaknesses();

        return new PlayerWeaknesses
        {
            WeakDefense = recentMoves.Count(m => m.MissedThreat) > recentMoves.Count * 0.3,
            WeakAttack = recentMoves.Count(m => m.MissedOpportunity) > recentMoves.Count * 0.3,
            InconsistentPlay = CalculateStandardDeviation(recentMoves.Select(m => m.Quality)) > 0.25,
            TooFast = recentMoves.Average(m => m.ThinkingTime) < 2000,
            TooSlow = recentMoves.Average(m => m.ThinkingTime) > 30000
        };
    }

    private double CalculateStandardDeviation(IEnumerable<double> values)
    {
        var valueList = values.ToList();
        if (valueList.Count == 0) return 0;

        double avg = valueList.Average();
        double sumOfSquares = valueList.Sum(v => Math.Pow(v - avg, 2));
        return Math.Sqrt(sumOfSquares / valueList.Count);
    }

    public void Reset()
    {
        moveHistory.Clear();
    }
}