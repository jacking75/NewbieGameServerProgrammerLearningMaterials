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
          ""DummyStartNumber"": 1,
          ""RemoteEndPoint"": ""127.0.0.1:32452"",
          ""ScenarioName"": ""BasicChatTest"",
          ""TestRunTimeMS"": 30000,
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
        /*string config = @"{
          ""DummyCount"": 100,
          ""DummyStartNumber"": 1,
          ""RemoteEndPoint"": ""127.0.0.1:32021"",
          ""ScenarioName"": ""BasicChatTest"",
          ""TaskConfigs"": [
            {
              ""TaskName"": ""Connect"",
              ""NextTaskProbabilityList"": [70, 30],
              ""NextTaskWaitMinTimeMSList"": [900, 900],
              ""NextTaskWaitMaxTimeMSList"": [1000, 1000],  
              ""NextTasks"": [""Login"", ""Disconnect""]
            },
            {
              ""TaskName"": ""Disconnect"",
              ""NextTaskProbabilityList"": [95, 5],
              ""NextTaskWaitMinTimeMSList"": [900, 900],
              ""NextTaskWaitMaxTimeMSList"": [1000, 1000],  
              ""NextTasks"": [""Connect"", ""TestEndCheck""]
            },
            {
              ""TaskName"": ""Login"",
              ""NextTaskProbabilityList"": [90, 10],
              ""NextTaskWaitMinTimeMSList"": [900, 900],
              ""NextTaskWaitMaxTimeMSList"": [1000, 1000],
              ""NextTasks"": [""EnterRoom"", ""Disconnect""]
            },
            {
              ""TaskName"": ""EnterRoom"",
              ""NextTaskProbabilityList"": [60, 40],
              ""NextTaskWaitMinTimeMSList"": [100, 100],
              ""NextTaskWaitMaxTimeMSList"": [2000, 2000],
              ""NextTasks"": [""Chat"", ""LeaveRoom""]
            },
            {
              ""TaskName"": ""LeaveRoom"",
              ""NextTaskProbabilityList"": [30, 70],
              ""NextTaskWaitMinTimeMSList"": [1000, 1500],
              ""NextTaskWaitMaxTimeMSList"": [1500, 2000],
              ""NextTasks"": [""Disonnect"", ""EnterRoom""]
            },
            {
              ""TaskName"": ""Chat"",
              ""NextTaskProbabilityList"": [60, 40],
              ""NextTaskWaitMinTimeMSList"": [2000, 1000],
              ""NextTaskWaitMaxTimeMSList"": [2500, 1500],
              ""NextTasks"": [""Chat"", ""LeaveRoom""]
            },
            {
              ""TaskName"": ""TestEndCheck"",
              ""NextTaskProbabilityList"": [100],
              ""NextTaskWaitMinTimeMSList"": [800],
              ""NextTaskWaitMaxTimeMSList"": [1000],
              ""NextTasks"": [""Disconnect""]
            }
          ]
        }";*/

        return config;
    }
}