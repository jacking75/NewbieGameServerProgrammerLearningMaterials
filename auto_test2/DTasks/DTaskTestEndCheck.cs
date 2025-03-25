using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Serilog;

using AutoTestClient.Dummy;


namespace AutoTestClient.DTasks;

public class DTaskTestEndCheck : DTask
{
    
    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
        _action = action;

        _endTime = DateTime.MaxValue;
    }

    public override async Task<DTaskResult> Run()
    {
        Log.Information($"[TestEndCheck Try] Dummy: {_runTimeData.DummyNumber}");


        var result = new DTaskResult();

        if (DateTime.Now < _endTime)    
        {
            result = MakeTaskResultComplete();
            return result;
        }


        result.Ret = DTaskResultValue.Terminated;
        result.NextDTaskIndex = 0;
        
        //Log.Debug($"Test End. endTime:{_endTime}");
        await Task.CompletedTask;
        Clear();

        Log.Information($"[TestEndCheck] Dummy: {_runTimeData.DummyNumber}");
        return result;
    }

    public override DTask Clone()
    {
        var task = new DTaskTestEndCheck();

        DeepCopy(task);
        task._endTime = _endTime;

        return task;
    }


    public override void Clear()
    {
    }

    public void SetEndTime(int endTimeMS)
    {
        // 현재 시간에 endTimeMS를 더한 시간을 _endTime에 저장한다.
        _endTime = DateTime.Now.AddMilliseconds(endTimeMS);
    }
}
