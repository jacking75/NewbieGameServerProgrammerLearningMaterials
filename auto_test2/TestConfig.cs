namespace AutoTestClient;

public class TestConfig
{
    public Int32 DummyCount { get; set; }

    // AutoTest 프로그램이 동시에 2개 이상 실행되었을 때 더미 이름이 중복되지 않도록 하기 위한 것인다.
    public Int32 DummyStartNumber { get; set; }

    public string ScenarioName { get; set; }

    // 테스트 실행 시간
    public Int32 TestRunTimeMS { get; set; } = 60000; // 60초

    public List<TaskConfig> TaskConfigs { get; set; }   
    
    public string RemoteEndPoint { get; set; }



    public ErrorCode Verify()
    {        
        if (DummyCount < 1)
        {
            return ErrorCode.InvalidDummyCount;
        }

        if (DummyStartNumber < 0)
        {
            return ErrorCode.InvalidDummyStartNumber;
        }
               

        return ErrorCode.None;
    }

    public (string, Int32) stringToIPPort()
    {
        var remoteInfos = RemoteEndPoint.Split(":");
        var ip = remoteInfos[0];
        var port = Int32.Parse(remoteInfos[1]);

        return (ip, port);
    }

    
}

public class TaskConfig
{
    public string TaskName { get; set; }
    public List<Int32> NextTaskProbabilityList { get; set; }
    public List<Int32> NextTaskWaitMinTimeMSList { get; set; }
    public List<Int32> NextTaskWaitMaxTimeMSList { get; set; }
    public List<string> NextTasks { get; set; }
}

