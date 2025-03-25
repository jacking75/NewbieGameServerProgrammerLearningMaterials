using Serilog;
using System.Text.Json;

namespace AutoTestClient;

// 프로그램은 닷넷이 설치된 PC에서 dotnet run 명령어로 실행하는 것을 가정한다.

public class Program
{
    private static void Main(string[] args)
    {
        bool isUseFileLog = false;
        InitLogger(isUseFileLog);
        
        Log.Information("[[[ Auto Test ]]]");

     
        var jsonConfig = JsonConfing();
        var testConfig = LoadConfig(jsonConfig);
        if (testConfig is null)
        {
            Log.Error("Failed load a config file");
            return;
        }

        TestConfigPrint.Print(testConfig);
        
        var runner = new TestRunner();
        runner.Init(testConfig);
        runner.Run().Wait();
    }


    private static void InitLogger(bool isUseFileLog)
    {
        var startTime = DateTime.Now.ToString("yyyyMMdd_HHmmss");

        if (isUseFileLog)
        {
            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Debug()
                .WriteTo.Console()
                .WriteTo.File($"logs/log_{startTime}.log", rollingInterval: RollingInterval.Day)
                .CreateLogger();
        }
        else
        {
            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Debug()
                .WriteTo.Console()
                .CreateLogger();
        }  
        
    }

    private static TestConfig LoadConfig(string jsonConfig)
    {
        TestConfig testConfig = JsonSerializer.Deserialize<TestConfig>(jsonConfig);                
        return testConfig;        
    }

    private static string JsonConfing()
    {
        string config = @"{
          ""DummyCount"": 2,
          ""DummyStartNumber"": 0,
          ""RemoteEndPoint"": ""127.0.0.1:32452"",
          ""ScenarioName"": ""BasicChatTest"",
          ""TestRunTimeMS"": 30000,
          ""MaxRoomCount"" : 8,
          ""RoomStartNumber"" : 1,
          ""RoomUserMaxCount"" : 2,
          ""TaskConfigs"": [
            {
              ""TaskName"": ""Connect"",
              ""NextTaskProbabilityList"": [100],
              ""NextTaskWaitMinTimeMSList"": [900],
              ""NextTaskWaitMaxTimeMSList"": [1000],  
              ""NextTasks"": [""Disconnect""]
            },
            {
              ""TaskName"": ""Disconnect"",
              ""NextTaskProbabilityList"": [10, 90],
              ""NextTaskWaitMinTimeMSList"": [900, 900],
              ""NextTaskWaitMaxTimeMSList"": [1000, 1000],  
              ""NextTasks"": [""Connect"", ""TestEndCheck""]
            },
            {
              ""TaskName"": ""TestEndCheck"",
              ""NextTaskProbabilityList"": [100],
              ""NextTaskWaitMinTimeMSList"": [800],
              ""NextTaskWaitMaxTimeMSList"": [1000],
              ""NextTasks"": [""Connect""]
            }
          ]
        }";
        
        return config;
    }
}