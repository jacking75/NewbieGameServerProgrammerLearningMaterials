
using OmokEngine.AI;
using OmokEngine.Core;

namespace OmokEngineTest;

/// <summary>
/// 오목 엔진 메인 클래스
/// </summary>
public class OmokEngineMain
{
    static void Main(string[] args)
    {
        Console.WriteLine("=== Complete Omok Engine ===\n");

        // 테스트 시나리오 선택
        Console.WriteLine("Select test mode:");
        Console.WriteLine("1. AI vs AI (Adaptive difficulty)");
        Console.WriteLine("2. Human vs AI (Training mode)");
        Console.WriteLine("3. Performance test");

        var choice = Console.ReadLine();

        switch (choice)
        {
            case "1":
                TestAIvsAI();
                break;
            case "2":
                TestHumanVsAI();
                break;
            case "3":
                TestPerformance();
                break;
            default:
                TestAIvsAI();
                break;
        }
    }

    /// <summary>
    /// AI vs AI 테스트
    /// </summary>
    static void TestAIvsAI()
    {
        Console.WriteLine("\n=== AI vs AI Test ===\n");

        var ai = new AdaptiveOmokAI(useRenjuRules: false);
        var board = ai.GetBoard();

        Stone currentPlayer = Stone.Black;
        int moveCount = 0;
        var startTime = DateTime.Now;

        while (moveCount < 100)
        {
            var move = ai.GetAIMove(currentPlayer, new Position(-1, -1), 3000);

            if (move == null)
            {
                Console.WriteLine("No valid moves!");
                break;
            }

            board.PlaceStone(move.Position, currentPlayer);
            moveCount++;

            Console.WriteLine($"Move {moveCount}: {currentPlayer} -> {move.Position} " +
                            $"(Score: {move.Score}, Type: {move.Type})");

            // 승리 체크
            if (board.CheckWin(move.Position, currentPlayer))
            {
                Console.WriteLine($"\n{currentPlayer} wins in {moveCount} moves!");
                Console.WriteLine($"Game duration: {(DateTime.Now - startTime).TotalSeconds:F2}s");
                PrintBoard(board);
                break;
            }

            // 플레이어 교체
            currentPlayer = currentPlayer == Stone.Black ? Stone.White : Stone.Black;

            // 10수마다 보드 출력
            if (moveCount % 10 == 0)
            {
                Console.WriteLine("\nCurrent board state:");
                PrintBoard(board);
                Console.WriteLine();
            }
        }

        var status = ai.GetStatus();
        Console.WriteLine($"\nFinal AI Status:");
        Console.WriteLine($"Difficulty: {status.CurrentDifficulty}");
    }

    /// <summary>
    /// 인간 vs AI 테스트
    /// </summary>
    static void TestHumanVsAI()
    {
        Console.WriteLine("\n=== Human vs AI Training Mode ===\n");
        Console.WriteLine("Enter moves as 'row,col' (e.g., '7,7'). Enter 'quit' to exit.\n");

        var ai = new AdaptiveOmokAI(useRenjuRules: false);
        var board = ai.GetBoard();

        Stone humanStone = Stone.Black;
        Stone aiStone = Stone.White;
        int moveCount = 0;

        PrintBoard(board);

        while (true)
        {
            // 인간 차례
            Console.Write($"\nYour move ({humanStone}): ");
            var input = Console.ReadLine();

            if (input?.ToLower() == "quit")
                break;

            if (!TryParseMove(input, out Position humanMove))
            {
                Console.WriteLine("Invalid input! Use format: row,col");
                continue;
            }

            if (!board.IsEmpty(humanMove.Row, humanMove.Col))
            {
                Console.WriteLine("Position already occupied!");
                continue;
            }

            var moveStart = DateTime.Now;
            board.PlaceStone(humanMove, humanStone);
            var thinkingTime = (long)(DateTime.Now - moveStart).TotalMilliseconds;
            moveCount++;

            PrintBoard(board);

            if (board.CheckWin(humanMove, humanStone))
            {
                Console.WriteLine("\nCongratulations! You won!");
                ai.RecordGameResult(false);
                ShowGameSummary(ai);
                break;
            }

            // AI 차례
            Console.WriteLine("\nAI is thinking...");
            var aiMove = ai.GetAIMove(aiStone, humanMove, thinkingTime);

            if (aiMove == null)
            {
                Console.WriteLine("No valid moves. Game draw!");
                break;
            }

            board.PlaceStone(aiMove.Position, aiStone);
            moveCount++;

            Console.WriteLine($"AI move: {aiMove.Position} (Type: {aiMove.Type})");
            PrintBoard(board);

            if (board.CheckWin(aiMove.Position, aiStone))
            {
                Console.WriteLine("\nAI won! Better luck next time.");
                ai.RecordGameResult(true);
                ShowGameSummary(ai);
                break;
            }

            // 상태 표시
            var status = ai.GetStatus();
            Console.WriteLine($"\nYour skill level: {status.PlayerSkillLevel} ({status.PlayerSkillScore:F1}/100)");
            Console.WriteLine($"AI difficulty: {status.CurrentDifficulty}");
        }
    }

    /// <summary>
    /// 성능 테스트
    /// </summary>
    static void TestPerformance()
    {
        Console.WriteLine("\n=== Performance Test ===\n");

        var ai = new AdaptiveOmokAI(useRenjuRules: false);
        var board = ai.GetBoard();

        // 몇 수 진행
        board.PlaceStone(new Position(7, 7), Stone.Black);
        board.PlaceStone(new Position(7, 8), Stone.White);
        board.PlaceStone(new Position(8, 7), Stone.Black);
        board.PlaceStone(new Position(8, 8), Stone.White);

        Console.WriteLine("Running 100 move evaluations...");
        var startTime = DateTime.Now;

        for (int i = 0; i < 100; i++)
        {
            var move = ai.GetAIMove(Stone.Black, new Position(8, 8), 3000);
        }

        var duration = (DateTime.Now - startTime).TotalMilliseconds;
        Console.WriteLine($"Total time: {duration:F2}ms");
        Console.WriteLine($"Average per move: {duration / 100:F2}ms");
    }

    /// <summary>
    /// 게임 요약 표시
    /// </summary>
    static void ShowGameSummary(AdaptiveOmokAI ai)
    {
        var status = ai.GetStatus();

        Console.WriteLine("\n========== Game Summary ==========");
        Console.WriteLine($"Your Skill Level: {status.PlayerSkillLevel}");
        Console.WriteLine($"Skill Score: {status.PlayerSkillScore:F1}/100");
        Console.WriteLine($"AI Difficulty: {status.CurrentDifficulty}");

        if (status.Weaknesses.WeakDefense)
            Console.WriteLine("💡 Tip: Focus more on defense and blocking opponent threats");

        if (status.Weaknesses.WeakAttack)
            Console.WriteLine("💡 Tip: Look for more aggressive attacking opportunities");

        if (status.Weaknesses.InconsistentPlay)
            Console.WriteLine("💡 Tip: Try to maintain consistent decision making");

        Console.WriteLine("==================================\n");
    }

    /// <summary>
    /// 보드 출력
    /// </summary>
    static void PrintBoard(OmokBoard board)
    {
        Console.Write("   ");
        for (int i = 0; i < board.GetBoardSize(); i++)
            Console.Write($"{i,3}");
        Console.WriteLine();

        for (int i = 0; i < board.GetBoardSize(); i++)
        {
            Console.Write($"{i,3}");
            for (int j = 0; j < board.GetBoardSize(); j++)
            {
                Stone stone = board.GetStone(i, j);
                string symbol = stone == Stone.Black ? " ●" :
                               stone == Stone.White ? " ○" : " ·";
                Console.Write($"{symbol} ");
            }
            Console.WriteLine();
        }
    }

    /// <summary>
    /// 입력 파싱
    /// </summary>
    static bool TryParseMove(string? input, out Position move)
    {
        move = new Position(-1, -1);

        if (string.IsNullOrWhiteSpace(input))
            return false;

        var parts = input.Split(',');
        if (parts.Length != 2)
            return false;

        if (int.TryParse(parts[0].Trim(), out int row) &&
            int.TryParse(parts[1].Trim(), out int col))
        {
            move = new Position(row, col);
            return true;
        }

        return false;
    }
}
