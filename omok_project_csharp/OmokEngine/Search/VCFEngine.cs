using OmokEngine.Core;
using OmokEngine.Evaluation;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Search;

/// <summary>
/// VCF (Victory by Continuous Fours) 탐색 엔진
/// </summary>
public class VCFEngine
{
    private OmokBoard board;
    private Dictionary<string, VCFResult> vcfCache;
    private int maxSearchDepth;

    public class VCFResult
    {
        public bool IsVCF { get; set; }
        public List<Position> WinningSequence { get; set; } = [];
        public int Depth { get; set; }
        public string Description { get; set; } = string.Empty;
    }

    public VCFEngine(OmokBoard board, int maxDepth = 12)
    {
        this.board = board;
        this.maxSearchDepth = maxDepth;
        this.vcfCache = new Dictionary<string, VCFResult>();
    }

    /// <summary>
    /// VCF 시퀀스 찾기
    /// </summary>
    public VCFResult FindVCFSequence(Stone attackerStone)
    {
        vcfCache.Clear();
        var sequence = new List<Position>();

        bool found = SearchVCF(
            attackerStone,
            maxSearchDepth,
            sequence,
            isAttackerTurn: true
        );

        return new VCFResult
        {
            IsVCF = found,
            WinningSequence = found ? new List<Position>(sequence) : new List<Position>(),
            Depth = sequence.Count,
            Description = found ? $"VCF found in {sequence.Count} moves" : "No VCF"
        };
    }

    private bool SearchVCF(Stone attackerStone, int depth, List<Position> sequence, bool isAttackerTurn)
    {
        if (depth <= 0)
            return false;

        // 보드 상태 해싱 (중복 계산 방지)
        string boardHash = GetBoardHash();
        if (vcfCache.ContainsKey(boardHash))
        {
            var cachedResult = vcfCache[boardHash];
            if (cachedResult.IsVCF)
            {
                sequence.AddRange(cachedResult.WinningSequence);
            }
            return cachedResult.IsVCF;
        }

        if (isAttackerTurn)
        {
            // 공격 차례: 4목을 만드는 수들 찾기
            var fourMoves = FindFourMoves(attackerStone);

            // 즉시 승리하는 5목이 있으면 반환
            foreach (var move in fourMoves)
            {
                board.PlaceStone(move, attackerStone);

                if (board.CheckWin(move, attackerStone))
                {
                    sequence.Add(move);
                    board.RemoveStone(move);

                    CacheResult(boardHash, true, new List<Position> { move });
                    return true;
                }

                board.RemoveStone(move);
            }

            // 열린 4목이 있으면 다음 수는 확정 승리
            var openFourMove = fourMoves.FirstOrDefault(m => IsOpenFour(m, attackerStone));
            if (openFourMove.Row >= 0)
            {
                sequence.Add(openFourMove);
                CacheResult(boardHash, true, new List<Position> { openFourMove });
                return true;
            }

            // 각 4목 후보에 대해 탐색
            foreach (var move in fourMoves.OrderByDescending(m => GetMovePriority(m, attackerStone)))
            {
                board.PlaceStone(move, attackerStone);
                sequence.Add(move);

                // 상대의 최선 방어를 시뮬레이션
                if (SearchVCF(attackerStone, depth - 1, sequence, false))
                {
                    board.RemoveStone(move);
                    CacheResult(boardHash, true, new List<Position>(sequence));
                    return true;
                }

                // 실패하면 되돌리기
                board.RemoveStone(move);
                sequence.RemoveAt(sequence.Count - 1);
            }

            CacheResult(boardHash, false, new List<Position>());
            return false;
        }
        else
        {
            // 방어 차례: 상대의 모든 4목 위협을 막아야 함
            Stone defenderStone = GetOpponentStone(attackerStone);
            var defensePositions = FindMandatoryDefenses(attackerStone);

            // 막을 수 없는 위협이 2개 이상이면 VCF 성공
            if (defensePositions.Count >= 2)
            {
                return true;
            }

            // 막을 수 없으면 VCF 성공
            if (defensePositions.Count == 0)
            {
                return true;
            }

            // 유일한 방어 수가 있으면 그 수를 두고 계속 탐색
            var defenseMove = defensePositions[0];
            board.PlaceStone(defenseMove, defenderStone);
            sequence.Add(defenseMove);

            bool result = SearchVCF(attackerStone, depth - 1, sequence, true);

            board.RemoveStone(defenseMove);

            if (!result)
            {
                // 방어에 성공하면 VCF 실패
                sequence.RemoveAt(sequence.Count - 1);
            }

            return result;
        }
    }

    private List<Position> FindFourMoves(Stone stone)
    {
        var fourMoves = new List<Position>();
        var candidates = GetCandidatePositions();

        foreach (var pos in candidates)
        {
            board.PlaceStone(pos, stone);

            var patterns = PatternAnalyzer.AnalyzePosition(board, pos, stone);

            foreach (var pattern in patterns.Values)
            {
                if (pattern.ConsecutiveStones >= 4)
                {
                    fourMoves.Add(pos);
                    break;
                }
            }

            board.RemoveStone(pos);
        }

        return fourMoves.Distinct().ToList();
    }

    private List<Position> FindMandatoryDefenses(Stone attackerStone)
    {
        var defensePositions = new List<Position>();
        var candidates = GetCandidatePositions();

        foreach (var pos in candidates)
        {
            board.PlaceStone(pos, attackerStone);

            // 이 위치에 공격자가 두면 5목이 되는가?
            if (board.CheckWin(pos, attackerStone))
            {
                board.RemoveStone(pos);
                defensePositions.Add(pos);
                continue;
            }

            // 열린 4목이 되는가?
            if (IsOpenFour(pos, attackerStone))
            {
                board.RemoveStone(pos);
                defensePositions.Add(pos);
                continue;
            }

            board.RemoveStone(pos);
        }

        return defensePositions;
    }

    private bool IsOpenFour(Position pos, Stone stone)
    {
        var patterns = PatternAnalyzer.AnalyzePosition(board, pos, stone);
        return patterns.Values.Any(p => p.ConsecutiveStones == 4 && p.OpenEnds == 2);
    }

    private int GetMovePriority(Position pos, Stone stone)
    {
        board.PlaceStone(pos, stone);
        var patterns = PatternAnalyzer.AnalyzePosition(board, pos, stone);
        board.RemoveStone(pos);

        int maxScore = 0;
        foreach (var pattern in patterns.Values)
        {
            maxScore = Math.Max(maxScore, PatternAnalyzer.CalculatePatternScore(pattern));
        }

        return maxScore;
    }

    private List<Position> GetCandidatePositions()
    {
        var positions = new List<Position>();
        int size = board.GetBoardSize();

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                var pos = new Position(i, j);
                if (board.IsEmpty(i, j) && board.HasNeighbor(pos, 2))
                {
                    positions.Add(pos);
                }
            }
        }

        return positions;
    }

    private string GetBoardHash()
    {
        var hash = new System.Text.StringBuilder();
        int size = board.GetBoardSize();

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                hash.Append((int)board.GetStone(i, j));
            }
        }

        return hash.ToString();
    }

    private void CacheResult(string hash, bool isVCF, List<Position> sequence)
    {
        vcfCache[hash] = new VCFResult
        {
            IsVCF = isVCF,
            WinningSequence = new List<Position>(sequence),
            Depth = sequence.Count
        };
    }

    private Stone GetOpponentStone(Stone stone)
    {
        return stone == Stone.Black ? Stone.White : Stone.Black;
    }
}