using OmokEngine.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.IO;

namespace OmokEngine.AI;

public class OpeningBook
{
    private Dictionary<string, List<OpeningMove>> openingDatabase = new();
    private readonly Random random = new();

    public class OpeningMove
    {
        public Position Move { get; set; }
        public string Name { get; set; } = string.Empty;
        public int WinRate { get; set; }  // 승률 (퍼센트)
        public string Description { get; set; } = string.Empty;
    }


    public OpeningBook()
    {
        InitializeOpenings();
    }

    private void InitializeOpenings()
    {
        openingDatabase = new Dictionary<string, List<OpeningMove>>();

        // 첫 수 (흑)
        openingDatabase[""] = new List<OpeningMove>
        {
            new OpeningMove
            {
                Move = new Position(7, 7),  // 중앙
                Name = "천원 (Tengen)",
                WinRate = 55,
                Description = "가장 기본적인 첫 수, 중앙 장악"
            }
        };

        // 백의 두 번째 수 (흑이 중앙에 둔 경우)
        openingDatabase["7,7"] = new List<OpeningMove>
        {
            new OpeningMove
            {
                Move = new Position(6, 6),
                Name = "대각선 접근",
                WinRate = 50,
                Description = "대각선으로 견제"
            },
            new OpeningMove
            {
                Move = new Position(7, 6),
                Name = "직접 접근",
                WinRate = 50,
                Description = "바로 옆에서 압박"
            },
            new OpeningMove
            {
                Move = new Position(6, 7),
                Name = "수직 접근",
                WinRate = 50,
                Description = "위아래로 견제"
            }
        };

        // 흑의 세 번째 수 예시들
        openingDatabase["7,7;6,6"] = new List<OpeningMove>
        {
            new OpeningMove
            {
                Move = new Position(8, 8),
                Name = "대각선 연장",
                WinRate = 52,
                Description = "대각선 라인 강화"
            },
            new OpeningMove
            {
                Move = new Position(7, 8),
                Name = "수평 확장",
                WinRate = 51,
                Description = "가로 방향 전개"
            }
        };

        // 실전에서는 더 많은 정석 추가
        LoadAdvancedOpenings();
    }

    private void LoadAdvancedOpenings()
    {
        // 유명한 오프닝 패턴들
        // 화월(花月), 포월(蒲月), 잔월(殘月) 등의 정석

        // 예: 화월 정석
        var huaYue = new List<OpeningMove>
        {
            new OpeningMove
            {
                Move = new Position(8, 6),
                Name = "화월 변화",
                WinRate = 53,
                Description = "화월 정석의 유력한 변화"
            }
        };
        openingDatabase["7,7;6,6;8,8"] = huaYue;

        // 더 많은 정석들을 추가...
        // 실제로는 외부 파일(JSON, DB)에서 로드하는 것이 좋음
    }

    // 오프닝 북에서 수 가져오기
    public Position? GetOpeningMove(List<Position> moveHistory)
    {
        if (moveHistory.Count > 6)  // 3수까지만 오프닝 북 사용
            return null;

        string key = GetHistoryKey(moveHistory);

        if (openingDatabase.TryGetValue(key, out var moves))
        {
            // 승률 기반 가중 랜덤 선택
            var selectedMove = SelectWeightedRandom(moves);
            return selectedMove?.Move;
        }

        return null;
    }

    private string GetHistoryKey(List<Position> history)
    {
        if (history.Count == 0)
            return "";

        return string.Join(";", history.Select(p => $"{p.Row},{p.Col}"));
    }

    private OpeningMove? SelectWeightedRandom(List<OpeningMove> moves)
    {
        if (moves.Count == 0)
            return null;

        int totalWeight = moves.Sum(m => m.WinRate);
        int randomValue = random.Next(totalWeight);
        int cumulative = 0;

        foreach (var move in moves)
        {
            cumulative += move.WinRate;
            if (randomValue < cumulative)
                return move;
        }

        return moves.Last();
    }

    // 외부 파일에서 오프닝 북 로드
    public void LoadFromFile(string filePath)
    {
        if (!File.Exists(filePath))
        {
            throw new FileNotFoundException($"오프닝 북 파일을 찾을 수 없습니다: {filePath}");
        }

        try
        {
            string jsonContent = File.ReadAllText(filePath);
            var loadedData = JsonSerializer.Deserialize<Dictionary<string, List<OpeningMove>>>(jsonContent);
            
            if (loadedData != null)
            {
                foreach (var kvp in loadedData)
                {
                    if (!openingDatabase.ContainsKey(kvp.Key))
                    {
                        openingDatabase[kvp.Key] = new List<OpeningMove>();
                    }
                    
                    openingDatabase[kvp.Key].AddRange(kvp.Value);
                }
            }
        }
        catch (JsonException ex)
        {
            throw new InvalidOperationException($"오프닝 북 파일 파싱 오류: {ex.Message}", ex);
        }
    }

    // 오프닝 북을 파일로 저장
    public void SaveToFile(string filePath)
    {
        try
        {
            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping
            };
            
            string jsonContent = JsonSerializer.Serialize(openingDatabase, options);
            File.WriteAllText(filePath, jsonContent);
        }
        catch (Exception ex)
        {
            throw new InvalidOperationException($"오프닝 북 파일 저장 오류: {ex.Message}", ex);
        }
    }

    // 오프닝 북에 새로운 패턴 추가 (학습 기능)
    public void AddOpening(List<Position> history, Position move, string name, int winRate)
    {
        string key = GetHistoryKey(history);

        if (!openingDatabase.ContainsKey(key))
        {
            openingDatabase[key] = new List<OpeningMove>();
        }

        openingDatabase[key].Add(new OpeningMove
        {
            Move = move,
            Name = name,
            WinRate = winRate,
            Description = ""
        });
    }
}