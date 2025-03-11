
using AutoTestClient.DTasks;
using Serilog;
using System.Collections.Generic;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace AutoTestClient.Dummy;

public class DummyManager
{
    public List<DummyObject> DummyList { get; private set; }

    public Dictionary<string, DummyObject> DummyDic { get; private set; }

    DTaskManager _taskManager = new();


    public void Init(TestConfig config)
    {
        SetTasks(config);

        DummyList = new(config.DummyCount);
        DummyDic = new(config.DummyCount);

        var startNumber = config.DummyStartNumber;

        for (int i = 0; i < DummyList.Capacity; ++i)
        {
            DummyObject dummy = new DummyObject();
            dummy.Init(i, startNumber + i, config, _taskManager.GetTaskList());

            DummyList.Add(dummy);
            DummyDic.Add(dummy.ID, dummy);
        }
    }

    public async Task<bool> Run()
    {
        List<Task<Dummy.DResult>> taskResult = new();

        foreach (var dummy in DummyList)
        {
            Log.Debug($"[Dummy Run] number: {dummy.Number}");
            taskResult.Add(dummy.Run());
        }

        Log.Debug($"[[All Dummy Task Wait]]");
        await Task.WhenAll(taskResult);

        Log.Information($"[[[All Dummy Task Terminate]]]");
        //TODO: 더미 결과를 출력한다. 출력을 하는 것은 Runner에서 하도록 델리게이트로 Runner에서 구현한 함수를 호출한다.
        return true;
    }

    bool SetTasks(TestConfig config)
    {
        _taskManager.Init(config);
        return true;
    }

    /*
    public async Task Action<T>(TestConfig config) where T : ScenarioBase, new()
    {
        var tasks = new List<Task>();

        foreach (AutoTestClient.Dummy.DummyObject dummy in DummyList)
        {
            var scenario = new T();

            scenario.Init(config);
            tasks.Add(scenario.Action(dummy));
        }

        await Task.WhenAll(tasks.ToArray());
    }

    public DummyObject GetDummyByIndex(int index)
    {
        try
        {
            return DummyList[index];
        }
        catch { return null; }
    }

    public DummyObject GetDummyByID(string id)
    {
        try
        {
            return DummyDic[id];
        }
        catch { return null; }
    }

    public int GetSuccessedActionCount()
    {
        int sum = 0;
        foreach (DummyObject dummy in DummyList)
        {
            sum += dummy.SucceededActionCount;
        }

        return sum;
    }

    public double GetAvgRTT()
    {
        long sum = 0;
        foreach (DummyObject dummy in DummyList)
        {
            sum += dummy.RTT;
        }

        return sum / DummyList.Capacity;
    }

    public void CheckingActionTimeout()
    {
        foreach (DummyObject dummy in DummyList)
        {
            if (dummy.IsScenarioRunning == false)
            {
                continue;
            }

            if (dummy.IsRunningActionTimeout() == true)
            {
                dummy.ScenarioDone(false, "Action Timeout");
            }
        }
    }

    public void Receive()
    {
        foreach (DummyObject dummy in DummyList)
        {
            if (dummy.IsScenarioRunning == false)
            {
                continue;
            }

            dummy.ReceiveAndAddPacketToPacketProcessor();
        }
    }*/
}