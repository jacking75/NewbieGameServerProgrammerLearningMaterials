using OmokEngine.Core;
using OmokEngine.Evaluation;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Search;

/// <summary>
/// 미니맥스 + 알파베타 + 트랜스포지션 테이블 엔진
/// </summary>
public class MinimaxEngine
{
    private OmokBoard board;
    private TranspositionTable ttable;
    private ZobristHasher hasher;
    private ulong currentHash;
    private int nodesEvaluated;

    public MinimaxEngine(OmokBoard board)
    {
        this.board = board;
        this.ttable = new TranspositionTable(256);  // 256MB
        this.hasher = new ZobristHasher();
        this.currentHash = hasher.ComputeHash(board);
    }

    /// <summary>
    /// 최선의 수 찾기
    /// </summary>
    public Position FindBestMove(Stone stone, int maxDepth, int timeLimitMs = 5000)
    {
        nodesEvaluated = 0;
        var startTime = DateTime.Now;

        Position bestMove = new Position(-1, -1);
        int bestScore = int.MinValue;

        var candidates = GetOrderedCandidates(stone);

        foreach (var candidate in candidates)
        {
            // 시간 제한 체크
            if ((DateTime.Now - startTime).TotalMilliseconds > timeLimitMs)
                break;

            // 수 두기
            board.PlaceStone(candidate, stone);
            currentHash = hasher.UpdateHash(currentHash, candidate, stone);

            // 즉시 승리
            if (board.CheckWin(candidate, stone))
            {
                currentHash = hasher.RemoveFromHash(currentHash, candidate, stone);
                board.RemoveStone(candidate);
                return candidate;
            }

            int score = -Negamax(
                GetOpponentStone(stone),
                maxDepth - 1,
                int.MinValue + 1,
                int.MaxValue - 1
            );

            // 수 되돌리기
            currentHash = hasher.RemoveFromHash(currentHash, candidate, stone);
            board.RemoveStone(candidate);

            if (score > bestScore)
            {
                bestScore = score;
                bestMove = candidate;
            }
        }

        return bestMove;
    }

    /// <summary>
    /// Negamax 알고리즘 (미니맥스의 간결한 버전)
    /// </summary>
    private int Negamax(Stone stone, int depth, int alpha, int beta)
    {
        nodesEvaluated++;
        int alphaOrig = alpha;

        // 트랜스포지션 테이블 조회
        if (ttable.Probe(currentHash, depth, alpha, beta, out int ttScore, out Position ttMove))
        {
            return ttScore;
        }

        // 종료 조건
        if (depth == 0)
        {
            return Evaluate(stone);
        }

        // Move Ordering: TT의 최선의 수를 먼저 탐색
        var candidates = GetOrderedCandidates(stone, ttMove);

        int bestScore = int.MinValue;
        Position bestMove = new Position(-1, -1);

        foreach (var candidate in candidates)
        {
            board.PlaceStone(candidate, stone);
            currentHash = hasher.UpdateHash(currentHash, candidate, stone);

            // 승리 체크
            if (board.CheckWin(candidate, stone))
            {
                currentHash = hasher.RemoveFromHash(currentHash, candidate, stone);
                board.RemoveStone(candidate);

                int winScore = 1000000 - depth;
                ttable.Store(
                    currentHash,
                    winScore,
                    depth,
                    candidate,
                    TranspositionTable.TTEntry.EntryType.Exact
                );

                return winScore;
            }

            int score = -Negamax(
                GetOpponentStone(stone),
                depth - 1,
                -beta,
                -alpha
            );

            currentHash = hasher.RemoveFromHash(currentHash, candidate, stone);
            board.RemoveStone(candidate);

            if (score > bestScore)
            {
                bestScore = score;
                bestMove = candidate;
            }

            alpha = Math.Max(alpha, score);

            // 베타 컷오프 (가지치기)
            if (alpha >= beta)
            {
                break;
            }
        }

        // 트랜스포지션 테이블에 저장
        TranspositionTable.TTEntry.EntryType entryType;
        if (bestScore <= alphaOrig)
            entryType = TranspositionTable.TTEntry.EntryType.UpperBound;
        else if (bestScore >= beta)
            entryType = TranspositionTable.TTEntry.EntryType.LowerBound;
        else
            entryType = TranspositionTable.TTEntry.EntryType.Exact;

        ttable.Store(currentHash, bestScore, depth, bestMove, entryType);

        return bestScore;
    }

    /// <summary>
    /// 보드 평가 함수
    /// </summary>
    private int Evaluate(Stone stone)
    {
        Stone opponent = GetOpponentStone(stone);

        int myScore = EvaluateForPlayer(stone);
        int opponentScore = EvaluateForPlayer(opponent);

        return myScore - opponentScore;
    }

    private int EvaluateForPlayer(Stone stone)
    {
        int score = 0;
        int size = board.GetBoardSize();

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                if (board.GetStone(i, j) == stone)
                {
                    var patterns = PatternAnalyzer.AnalyzePosition(
                        board, new Position(i, j), stone);

                    foreach (var pattern in patterns.Values)
                    {
                        score += PatternAnalyzer.CalculatePatternScore(pattern);
                    }
                }
            }
        }

        return score / 4;  // 4방향 중복 제거
    }

    /// <summary>
    /// 정렬된 후보 수들 가져오기 (Move Ordering)
    /// </summary>
    private List<Position> GetOrderedCandidates(Stone stone, Position ttMove = default)
    {
        var evaluator = new MoveEvaluator(board);
        var candidates = evaluator.EvaluateAllMoves(stone, 20);

        // TT의 최선의 수를 맨 앞으로
        if (ttMove.Row >= 0)
        {
            candidates = candidates
                .OrderByDescending(c => c.Position.Equals(ttMove) ? int.MaxValue : c.Score)
                .ToList();
        }

        return candidates.Select(c => c.Position).ToList();
    }

    private Stone GetOpponentStone(Stone stone)
    {
        return stone == Stone.Black ? Stone.White : Stone.Black;
    }

    public void ClearCache()
    {
        ttable.Clear();
        currentHash = hasher.ComputeHash(board);
    }

    public double GetCacheUsage()
    {
        return ttable.GetUsagePercent();
    }

    public int GetNodesEvaluated() => nodesEvaluated;
}
