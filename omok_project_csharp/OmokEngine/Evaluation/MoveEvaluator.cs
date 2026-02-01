using OmokEngine.Core;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Evaluation;

/// <summary>
/// 수 평가 엔진
/// </summary>
public class MoveEvaluator
{
    private OmokBoard board;

    public MoveEvaluator(OmokBoard board)
    {
        this.board = board;
    }

    /// <summary>
    /// 모든 가능한 수를 평가
    /// </summary>
    public List<EvaluatedMove> EvaluateAllMoves(Stone playerStone, int maxCandidates = 20)
    {
        var evaluatedMoves = new List<EvaluatedMove>();
        Stone opponentStone = GetOpponentStone(playerStone);

        // 첫 수인 경우 중앙 반환
        if (board.GetMoveHistory().Count == 0)
        {
            int center = board.GetBoardSize() / 2;
            return new List<EvaluatedMove>
                {
                    new EvaluatedMove(new Position(center, center), 10000, MoveType.Strategic, "Opening move")
                };
        }

        // 가능한 모든 위치를 평가 (주변에 돌이 있는 곳만)
        var candidates = GetCandidatePositions();

        foreach (var pos in candidates)
        {
            int score = EvaluatePosition(pos, playerStone, opponentStone, out MoveType moveType);
            evaluatedMoves.Add(new EvaluatedMove(pos, score, moveType));
        }

        // 점수순으로 정렬하고 상위 N개만 반환
        return evaluatedMoves
            .OrderByDescending(m => m.Score)
            .Take(maxCandidates)
            .ToList();
    }

    /// <summary>
    /// 후보 위치 추출 (최적화: 주변에 돌이 있는 위치만)
    /// </summary>
    private List<Position> GetCandidatePositions()
    {
        var candidates = new List<Position>();
        int size = board.GetBoardSize();

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                var pos = new Position(i, j);
                if (board.IsEmpty(i, j) && board.HasNeighbor(pos, 2))
                {
                    candidates.Add(pos);
                }
            }
        }

        return candidates;
    }

    /// <summary>
    /// 특정 위치의 점수 평가
    /// </summary>
    private int EvaluatePosition(Position pos, Stone playerStone, Stone opponentStone, out MoveType moveType)
    {
        moveType = MoveType.Neutral;
        int totalScore = 0;

        // 시뮬레이션: 실제로 돌을 놓고 평가
        board.PlaceStone(pos, playerStone);

        // 승리 체크
        if (board.CheckWin(pos, playerStone))
        {
            board.RemoveStone(pos);
            moveType = MoveType.Winning;
            return 1000000;  // 최고 점수
        }

        // 공격 점수 계산
        var playerPatterns = PatternAnalyzer.AnalyzePosition(board, pos, playerStone);
        int attackScore = CalculateTotalScore(playerPatterns);

        board.RemoveStone(pos);

        // 방어 점수 계산 (상대가 이 위치에 두었을 때)
        board.PlaceStone(pos, opponentStone);

        // 상대 승리 방어
        if (board.CheckWin(pos, opponentStone))
        {
            board.RemoveStone(pos);
            moveType = MoveType.DefendFour;
            return 900000;  // 승리 다음으로 중요
        }

        var opponentPatterns = PatternAnalyzer.AnalyzePosition(board, pos, opponentStone);
        int defenseScore = CalculateTotalScore(opponentPatterns);

        board.RemoveStone(pos);

        // 방어가 공격보다 약간 더 중요 (1.1배 가중치)
        totalScore = attackScore + (int)(defenseScore * 1.1);

        // 수 타입 결정
        moveType = DetermineMoveType(playerPatterns, opponentPatterns);

        // 위치 보너스 (중앙에 가까울수록 약간 유리)
        totalScore += GetPositionBonus(pos);

        return totalScore;
    }

    /// <summary>
    /// 패턴들의 총 점수 계산
    /// </summary>
    private int CalculateTotalScore(Dictionary<string, Pattern> patterns)
    {
        int totalScore = 0;

        foreach (var pattern in patterns.Values)
        {
            totalScore += PatternAnalyzer.CalculatePatternScore(pattern);
        }

        return totalScore;
    }

    /// <summary>
    /// 수의 타입 결정
    /// </summary>
    private MoveType DetermineMoveType(Dictionary<string, Pattern> playerPatterns,
                                       Dictionary<string, Pattern> opponentPatterns)
    {
        // 상대방 패턴 체크 (방어)
        foreach (var pattern in opponentPatterns.Values)
        {
            if (pattern.ConsecutiveStones == 4)
                return MoveType.DefendFour;
            if (pattern.ConsecutiveStones == 3 && pattern.OpenEnds == 2)
                return MoveType.DefendThree;
        }

        // 자신의 패턴 체크 (공격)
        foreach (var pattern in playerPatterns.Values)
        {
            if (pattern.ConsecutiveStones == 4)
                return MoveType.MakeFour;
            if (pattern.ConsecutiveStones == 3 && pattern.OpenEnds == 2)
                return MoveType.MakeThree;
        }

        return MoveType.Strategic;
    }

    /// <summary>
    /// 위치 보너스 (중앙 선호)
    /// </summary>
    private int GetPositionBonus(Position pos)
    {
        int center = board.GetBoardSize() / 2;
        int distance = Math.Abs(pos.Row - center) + Math.Abs(pos.Col - center);
        return Math.Max(0, 50 - distance * 2);
    }

    private Stone GetOpponentStone(Stone stone)
    {
        return stone == Stone.Black ? Stone.White : Stone.Black;
    }
}