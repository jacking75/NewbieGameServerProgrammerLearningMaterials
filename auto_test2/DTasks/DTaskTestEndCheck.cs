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
        var result = new DTaskResult();

        if (DateTime.Now < _endTime)    
        {
            result.Ret = DTaskResultValue.Completed;
            (result.NextDTaskWaitTimeMS, result.NextDTaskIndex) = NextTask();
            return result;
        }


        result.Ret = DTaskResultValue.Terminated;
        result.NextDTaskIndex = 0;
        
        Log.Information($"Test End. endTime:{_endTime}");
        await Task.CompletedTask;

        Clear();
        return result;
    }

    public override DTask Clone()
    {
        var task = new DTaskTestEndCheck();

        DeepCopy(task);
        task._endTime = _endTime;

        return task;
    }

    
    protected override void Clear()
    {
    }

    public void SetEndTime(int endTimeMS)
    {
        // 현재 시간에 endTimeMS를 더한 시간을 _endTime에 저장한다.
        _endTime = DateTime.Now.AddMilliseconds(endTimeMS);
    }
}
