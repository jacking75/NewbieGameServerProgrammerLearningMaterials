using OmokEngine.Analysis;
using OmokEngine.Core;
using OmokEngine.Evaluation;
using OmokEngine.Search;
using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.AI;

/// <summary>
/// 적응형 오목 AI
/// </summary>
public class AdaptiveOmokAI
{
    private OmokBoard board;
    private MinimaxEngine minimaxEngine;
    private VCFEngine vcfEngine;
    private RenjuRuleChecker renjuChecker;
    public PlayerSkillAnalyzer analyzer;
    private DifficultyConfig currentConfig;
    private Random random;

    private int consecutiveWins = 0;
    private int consecutiveLosses = 0;
    private bool useRenjuRules;

    public class DifficultyConfig
    {
        public double OptimalMoveProb { get; set; }
        public double GoodMoveProb { get; set; }
        public double MistakeProbability { get; set; }
        public bool IgnoreCriticalThreats { get; set; }
        public int SearchDepth { get; set; }
        public string Description { get; set; } = string.Empty;
    }

    public AdaptiveOmokAI(bool useRenjuRules = false)
    {
        board = new OmokBoard();
        minimaxEngine = new MinimaxEngine(board);
        vcfEngine = new VCFEngine(board, 12);
        renjuChecker = new RenjuRuleChecker(board);
        analyzer = new PlayerSkillAnalyzer();
        random = new Random();
        this.useRenjuRules = useRenjuRules;
        currentConfig = GetConfigForSkillLevel(PlayerSkillLevel.Beginner);
    }

    /// <summary>
    /// AI의 수 선택
    /// </summary>
    public EvaluatedMove GetAIMove(Stone aiStone, Position lastPlayerMove, long playerThinkingTime)
    {
        // 플레이어의 마지막 수 분석
        if (lastPlayerMove.Row >= 0 && lastPlayerMove.Col >= 0)
        {
            var moveQuality = analyzer.AnalyzePlayerMove(
                board, lastPlayerMove,
                GetOpponentStone(aiStone),
                playerThinkingTime
            );

            // 실력 레벨 업데이트
            UpdateDifficulty();
        }

        // 1. VCF 탐색
        var vcfResult = vcfEngine.FindVCFSequence(aiStone);
        if (vcfResult.IsVCF && random.NextDouble() < currentConfig.OptimalMoveProb)
        {
            return new EvaluatedMove(
                vcfResult.WinningSequence[0],
                1000000,
                MoveType.Winning,
                "VCF sequence"
            );
        }

        // 2. 후보 수들 가져오기
        var topMoves = GetTopMoves(aiStone, 15);

        if (topMoves.Count == 0)
            return null!;

        // 3. 적응형 수 선택
        return SelectMoveAdaptively(topMoves, aiStone);
    }

    private List<EvaluatedMove> GetTopMoves(Stone aiStone, int count)
    {
        // 기본 평가
        var evaluator = new MoveEvaluator(board);
        var moves = evaluator.EvaluateAllMoves(aiStone, count);

        // 렌주 규칙 필터링
        if (useRenjuRules && aiStone == Stone.Black)
        {
            moves = moves.Where(m =>
            {
                var info = renjuChecker.CheckForbiddenMove(m.Position, aiStone);
                return info.IsAllowed;
            }).ToList();
        }

        return moves;
    }

    private EvaluatedMove SelectMoveAdaptively(List<EvaluatedMove> moves, Stone aiStone)
    {
        // 1단계: 치명적인 수는 항상 고려 (승리, 상대 4목 막기)
        var criticalMove = moves.FirstOrDefault(m =>
            m.Type == MoveType.Winning ||
            m.Type == MoveType.DefendFour);

        if (criticalMove != null)
        {
            // 초급자 상대로는 가끔 일부러 놓침 (학습 기회 제공)
            if (currentConfig.IgnoreCriticalThreats == false || random.NextDouble() > 0.1)
                return criticalMove;
        }

        // 2단계: 난이도 설정에 따른 확률적 선택
        double rand = random.NextDouble();

        if (rand < currentConfig.OptimalMoveProb)
        {
            // 최선의 수
            return moves[0];
        }
        else if (rand < currentConfig.OptimalMoveProb + currentConfig.GoodMoveProb)
        {
            // 좋은 수 (2-4위)
            int index = random.Next(1, Math.Min(4, moves.Count));
            return moves[index];
        }
        else
        {
            // 실수 수준의 수 (5-10위)
            int index = random.Next(4, Math.Min(10, moves.Count));
            return moves[index];
        }
    }

    /// <summary>
    /// 난이도 동적 조절
    /// </summary>
    private void UpdateDifficulty()
    {
        var currentLevel = analyzer.GetCurrentSkillLevel();
        var weaknesses = analyzer.AnalyzeWeaknesses();

        // 기본 설정 업데이트
        currentConfig = GetConfigForSkillLevel(currentLevel);

        // 연승/연패에 따른 미세 조정
        if (consecutiveWins >= 3)
        {
            currentConfig.OptimalMoveProb = Math.Min(0.95, currentConfig.OptimalMoveProb + 0.10);
            currentConfig.MistakeProbability = Math.Max(0.01, currentConfig.MistakeProbability - 0.05);
        }
        else if (consecutiveLosses >= 3)
        {
            currentConfig.OptimalMoveProb = Math.Max(0.15, currentConfig.OptimalMoveProb - 0.10);
            currentConfig.MistakeProbability = Math.Min(0.50, currentConfig.MistakeProbability + 0.10);
        }

        // 플레이어의 약점에 따른 조정
        if (weaknesses.WeakDefense)
        {
            currentConfig.OptimalMoveProb *= 0.95;
        }

        if (weaknesses.WeakAttack)
        {
            currentConfig.MistakeProbability += 0.05;
        }
    }

    private DifficultyConfig GetConfigForSkillLevel(PlayerSkillLevel level)
    {
        return level switch
        {
            PlayerSkillLevel.Novice => new DifficultyConfig
            {
                OptimalMoveProb = 0.20,
                GoodMoveProb = 0.40,
                MistakeProbability = 0.40,
                IgnoreCriticalThreats = false,
                SearchDepth = 1,
                Description = "Very Easy"
            },
            PlayerSkillLevel.Beginner => new DifficultyConfig
            {
                OptimalMoveProb = 0.35,
                GoodMoveProb = 0.45,
                MistakeProbability = 0.20,
                IgnoreCriticalThreats = false,
                SearchDepth = 1,
                Description = "Easy"
            },
            PlayerSkillLevel.Intermediate => new DifficultyConfig
            {
                OptimalMoveProb = 0.50,
                GoodMoveProb = 0.35,
                MistakeProbability = 0.15,
                IgnoreCriticalThreats = false,
                SearchDepth = 2,
                Description = "Medium"
            },
            PlayerSkillLevel.Advanced => new DifficultyConfig
            {
                OptimalMoveProb = 0.70,
                GoodMoveProb = 0.25,
                MistakeProbability = 0.05,
                IgnoreCriticalThreats = false,
                SearchDepth = 3,
                Description = "Hard"
            },
            PlayerSkillLevel.Expert => new DifficultyConfig
            {
                OptimalMoveProb = 0.85,
                GoodMoveProb = 0.13,
                MistakeProbability = 0.02,
                IgnoreCriticalThreats = false,
                SearchDepth = 4,
                Description = "Expert"
            },
            _ => new DifficultyConfig
            {
                OptimalMoveProb = 0.50,
                GoodMoveProb = 0.35,
                MistakeProbability = 0.15,
                IgnoreCriticalThreats = false,
                SearchDepth = 2,
                Description = "Default"
            }
        };
    }

    /// <summary>
    /// 게임 종료 후 결과 기록
    /// </summary>
    public void RecordGameResult(bool aiWon)
    {
        if (aiWon)
        {
            consecutiveWins++;
            consecutiveLosses = 0;
        }
        else
        {
            consecutiveLosses++;
            consecutiveWins = 0;
        }
    }

    public void StartNewGame()
    {
        board.Clear();
        minimaxEngine.ClearCache();
    }

    public void ResetAll()
    {
        board.Clear();
        minimaxEngine.ClearCache();
        analyzer.Reset();
        consecutiveWins = 0;
        consecutiveLosses = 0;
        currentConfig = GetConfigForSkillLevel(PlayerSkillLevel.Beginner);
    }

    public OmokBoard GetBoard() => board;

    public AIStatus GetStatus()
    {
        return new AIStatus
        {
            PlayerSkillLevel = analyzer.GetCurrentSkillLevel(),
            PlayerSkillScore = analyzer.GetSkillScore(),
            CurrentDifficulty = currentConfig.Description,
            ConsecutiveWins = consecutiveWins,
            ConsecutiveLosses = consecutiveLosses,
            Weaknesses = analyzer.AnalyzeWeaknesses()
        };
    }

    private Stone GetOpponentStone(Stone stone)
    {
        return stone == Stone.Black ? Stone.White : Stone.Black;
    }
}

public class AIStatus
{
    public PlayerSkillLevel PlayerSkillLevel { get; set; }
    public double PlayerSkillScore { get; set; }
    public string CurrentDifficulty { get; set; } = string.Empty;
    public int ConsecutiveWins { get; set; }
    public int ConsecutiveLosses { get; set; }
    public PlayerWeaknesses Weaknesses { get; set; } = new();
}
