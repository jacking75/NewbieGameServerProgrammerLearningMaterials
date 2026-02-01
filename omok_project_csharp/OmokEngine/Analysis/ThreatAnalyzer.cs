using OmokEngine.Core;
using OmokEngine.Evaluation;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Analysis;

public class ThreatAnalyzer
{
    private OmokBoard board;

    public ThreatAnalyzer(OmokBoard board)
    {
        this.board = board;
    }

    public class Threat
    {
        public Position AttackPosition { get; set; }     // 공격할 위치
        public Position DefensePosition { get; set; }    // 막아야 할 위치
        public ThreatType Type { get; set; }
        public Stone Attacker { get; set; }
        public int Severity { get; set; }                // 위협도 (높을수록 위험)
        public List<Position> RelatedPositions { get; set; } = [];  // 관련된 돌들
    }

    public enum ThreatType
    {
        Five,              // 5목 (즉시 승리)
        OpenFour,          // 열린 4목 (막아도 다음 수 승리)
        Four,              // 4목
        DoubleThree,       // 쌍삼 (3목 2개 동시)
        OpenThree,         // 열린 3목
        BrokenThree,       // 띄어진 3목
        DoubleAttack       // 멀티 공격
    }

    // 모든 위협 분석
    public List<Threat> AnalyzeAllThreats(Stone attackerStone)
    {
        var threats = new List<Threat>();

        var candidates = GetRelevantPositions();

        foreach (var pos in candidates)
        {
            if (!board.IsEmpty(pos.Row, pos.Col))
                continue;

            // 이 위치에 돌을 놓았을 때의 위협 분석
            board.PlaceStone(pos, attackerStone);

            var localThreats = AnalyzePositionThreats(pos, attackerStone);
            threats.AddRange(localThreats);

            board.RemoveStone(pos);
        }

        return threats.OrderByDescending(t => t.Severity).ToList();
    }

    private List<Threat> AnalyzePositionThreats(Position pos, Stone stone)
    {
        var threats = new List<Threat>();

        // 5목 체크
        if (board.CheckWin(pos, stone))
        {
            threats.Add(new Threat
            {
                AttackPosition = pos,
                DefensePosition = pos,
                Type = ThreatType.Five,
                Attacker = stone,
                Severity = 100000,
                RelatedPositions = GetRelatedStones(pos, stone)
            });
            return threats;
        }

        // 패턴 분석
        var patterns = PatternAnalyzer.AnalyzePosition(board, pos, stone);

        foreach (var kvp in patterns)
        {
            var pattern = kvp.Value;

            // 열린 4목
            if (pattern.ConsecutiveStones == 4 && pattern.OpenEnds == 2)
            {
                threats.Add(new Threat
                {
                    AttackPosition = pos,
                    DefensePosition = pos,
                    Type = ThreatType.OpenFour,
                    Attacker = stone,
                    Severity = 50000,
                    RelatedPositions = GetRelatedStones(pos, stone)
                });
            }
            // 4목
            else if (pattern.ConsecutiveStones == 4 && pattern.OpenEnds == 1)
            {
                threats.Add(new Threat
                {
                    AttackPosition = pos,
                    DefensePosition = pos,
                    Type = ThreatType.Four,
                    Attacker = stone,
                    Severity = 10000,
                    RelatedPositions = GetRelatedStones(pos, stone)
                });
            }
            // 열린 3목
            else if (pattern.ConsecutiveStones == 3 && pattern.OpenEnds == 2)
            {
                threats.Add(new Threat
                {
                    AttackPosition = pos,
                    DefensePosition = pos,
                    Type = ThreatType.OpenThree,
                    Attacker = stone,
                    Severity = 5000,
                    RelatedPositions = GetRelatedStones(pos, stone)
                });
            }
        }

        // 쌍삼 (Double Three) 체크
        var openThrees = threats.Where(t => t.Type == ThreatType.OpenThree).ToList();
        if (openThrees.Count >= 2)
        {
            threats.Add(new Threat
            {
                AttackPosition = pos,
                DefensePosition = pos,
                Type = ThreatType.DoubleThree,
                Attacker = stone,
                Severity = 30000,  // 쌍삼은 매우 강력
                RelatedPositions = GetRelatedStones(pos, stone)
            });
        }

        return threats;
    }

    // 필승 시퀀스 찾기 (VCF: Victory by Continuous Fours)
    public List<Position>? FindVCFSequence(Stone attackerStone, int maxDepth = 10)
    {
        var sequence = new List<Position>();
        return FindVCFRecursive(attackerStone, maxDepth, sequence) ? sequence : null;
    }

    private bool FindVCFRecursive(Stone attackerStone, int depth, List<Position> sequence)
    {
        if (depth == 0)
            return false;

        var threats = AnalyzeAllThreats(attackerStone);

        // 즉시 승리
        var winningMove = threats.FirstOrDefault(t => t.Type == ThreatType.Five);
        if (winningMove != null)
        {
            sequence.Add(winningMove.AttackPosition);
            return true;
        }

        // 4목 만들기
        var fourMoves = threats.Where(t =>
            t.Type == ThreatType.Four ||
            t.Type == ThreatType.OpenFour).ToList();

        foreach (var fourMove in fourMoves)
        {
            board.PlaceStone(fourMove.AttackPosition, attackerStone);
            sequence.Add(fourMove.AttackPosition);

            // 상대의 방어 수 시뮬레이션
            var defenseThreats = AnalyzeAllThreats(attackerStone);
            var criticalDefense = defenseThreats
                .Where(t => t.Type == ThreatType.Four || t.Type == ThreatType.OpenFour)
                .OrderByDescending(t => t.Severity)
                .FirstOrDefault();

            if (criticalDefense != null)
            {
                Stone opponentStone = GetOpponentStone(attackerStone);
                board.PlaceStone(criticalDefense.DefensePosition, opponentStone);

                // 재귀적으로 다음 4목 찾기
                if (FindVCFRecursive(attackerStone, depth - 1, sequence))
                {
                    board.RemoveStone(criticalDefense.DefensePosition);
                    board.RemoveStone(fourMove.AttackPosition);
                    return true;
                }

                board.RemoveStone(criticalDefense.DefensePosition);
            }
            else
            {
                // 방어할 수 없으면 승리
                board.RemoveStone(fourMove.AttackPosition);
                return true;
            }

            board.RemoveStone(fourMove.AttackPosition);
            sequence.RemoveAt(sequence.Count - 1);
        }

        return false;
    }

    // 관련된 돌들의 위치 찾기
    private List<Position> GetRelatedStones(Position center, Stone stone)
    {
        var related = new List<Position>();
        var directions = new[] { (0, 1), (1, 0), (1, 1), (1, -1) };

        foreach (var (dx, dy) in directions)
        {
            // 양방향 탐색
            for (int dir = -1; dir <= 1; dir += 2)
            {
                int row = center.Row + dx * dir;
                int col = center.Col + dy * dir;

                while (board.IsValidPosition(row, col) && board.GetStone(row, col) == stone)
                {
                    related.Add(new Position(row, col));
                    row += dx * dir;
                    col += dy * dir;
                }
            }
        }

        return related;
    }

    private List<Position> GetRelevantPositions()
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

    private Stone GetOpponentStone(Stone stone)
    {
        return stone == Stone.Black ? Stone.White : Stone.Black;
    }
}