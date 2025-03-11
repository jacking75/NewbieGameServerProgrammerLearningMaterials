
namespace AutoTestClient;

public class TestRunner
{
    Dummy.DummyManager _dummyMgr = new();
        
    TestConfig _config;
    

    public void Init(TestConfig config)
    {
        _config = config;
        
        _dummyMgr.Init(config);

        AutoTestMonitor.Instance.Init();        
    }

    public async Task<bool> Run()
    {
        Prepare();

        var ret = await _dummyMgr.Run();
                
        Done();

        Record();

        return ret;
    }

    void Record()
    {
        /*
        var totalActionCount = _dummyMgr.GetSuccessedActionCount();
        var failedCount = 0;
        var succeededCount = 0;
        var elapsedTime = _endTime - _startTime;

        Console.WriteLine($"--------------------------------------------------------------------------------");
        Console.WriteLine($"[{DateTime.Now}]");
        foreach (var dummy in _dummyMgr.DummyList)
        {
            var result = dummy.ScenarioResult;

            if (result.IsSucceeded == false)
            {
                ++failedCount;
            }
            else
            {
                ++succeededCount;
            }

            Console.WriteLine($"Number: {dummy.Number}, IsSucceeded: {result.IsSucceeded}, Message: {result.Message}");
        }

        Console.WriteLine($"\nScenarioType: {_config.Scenario.Value} ElapsedTime: {elapsedTime.TotalMilliseconds}ms, TotalActionCount: {totalActionCount}, Succeeded Dummy Count: {succeededCount}, Failed Dummy Count: {failedCount}");
        Console.WriteLine($"--------------------------------------------------------------------------------");*/
    }

    private void Prepare()
    {        
    }

    private void Done()
    {
        AutoTestMonitor.Instance.EndTest();
    }
}
