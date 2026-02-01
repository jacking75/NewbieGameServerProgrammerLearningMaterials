using OmokEngine.Core;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Search;

/// <summary>
/// 트랜스포지션 테이블 (중복 계산 방지)
/// </summary>
public class TranspositionTable
{
    private Dictionary<ulong, TTEntry> table;
    private int maxEntries;

    public class TTEntry
    {
        public ulong Hash { get; set; }
        public int Score { get; set; }
        public int Depth { get; set; }
        public Position BestMove { get; set; }
        public EntryType Type { get; set; }
        public DateTime Timestamp { get; set; }

        public enum EntryType
        {
            Exact,      // 정확한 값
            LowerBound, // 알파 컷오프 (실제 값은 이보다 높음)
            UpperBound  // 베타 컷오프 (실제 값은 이보다 낮음)
        }
    }

    public TranspositionTable(int maxSizeMB = 128)
    {
        // MB를 엔트리 개수로 변환 (각 엔트리는 약 40바이트)
        maxEntries = (maxSizeMB * 1024 * 1024) / 40;
        table = new Dictionary<ulong, TTEntry>(maxEntries);
    }

    /// <summary>
    /// 엔트리 저장
    /// </summary>
    public void Store(ulong hash, int score, int depth, Position bestMove, TTEntry.EntryType type)
    {
        // 테이블이 가득 찼으면 오래된 엔트리 제거
        if (table.Count >= maxEntries && !table.ContainsKey(hash))
        {
            CleanOldEntries();
        }

        // 기존 엔트리가 있고 더 깊은 탐색이었으면 유지
        if (table.TryGetValue(hash, out var existing))
        {
            if (existing.Depth > depth && existing.Type == TTEntry.EntryType.Exact)
            {
                return;  // 더 정확한 기존 값 유지
            }
        }

        table[hash] = new TTEntry
        {
            Hash = hash,
            Score = score,
            Depth = depth,
            BestMove = bestMove,
            Type = type,
            Timestamp = DateTime.Now
        };
    }

    /// <summary>
    /// 엔트리 조회
    /// </summary>
    public bool Probe(ulong hash, int depth, int alpha, int beta, out int score, out Position bestMove)
    {
        score = 0;
        bestMove = new Position(-1, -1);

        if (!table.TryGetValue(hash, out var entry))
            return false;

        // 저장된 탐색 깊이가 현재보다 얕으면 사용 불가
        if (entry.Depth < depth)
            return false;

        bestMove = entry.BestMove;

        // 엔트리 타입에 따라 사용 가능 여부 판단
        switch (entry.Type)
        {
            case TTEntry.EntryType.Exact:
                score = entry.Score;
                return true;

            case TTEntry.EntryType.LowerBound:
                if (entry.Score >= beta)
                {
                    score = entry.Score;
                    return true;
                }
                break;

            case TTEntry.EntryType.UpperBound:
                if (entry.Score <= alpha)
                {
                    score = entry.Score;
                    return true;
                }
                break;
        }

        return false;
    }

    /// <summary>
    /// 최선의 수만 가져오기 (Move Ordering용)
    /// </summary>
    public Position GetBestMove(ulong hash)
    {
        if (table.TryGetValue(hash, out var entry))
        {
            return entry.BestMove;
        }
        return new Position(-1, -1);
    }

    /// <summary>
    /// 오래된 엔트리 제거
    /// </summary>
    private void CleanOldEntries()
    {
        var cutoffTime = DateTime.Now.AddSeconds(-30);
        var oldEntries = table.Where(kvp => kvp.Value.Timestamp < cutoffTime)
                              .Select(kvp => kvp.Key)
                              .Take(maxEntries / 4)  // 25% 제거
                              .ToList();

        foreach (var key in oldEntries)
        {
            table.Remove(key);
        }
    }

    public void Clear()
    {
        table.Clear();
    }

    public int GetSize()
    {
        return table.Count;
    }

    public double GetUsagePercent()
    {
        return (double)table.Count / maxEntries * 100;
    }
}
