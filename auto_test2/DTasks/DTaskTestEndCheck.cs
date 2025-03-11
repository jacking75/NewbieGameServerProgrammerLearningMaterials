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
    DateTime _endTime;

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
            (result.NextDTaskWaitTimeMS, result.NextDTask) = NextTask();
            return result;
        }


        result.Ret = DTaskResultValue.Terminated;
        result.NextDTask = null;
        Clear();

        Log.Information("Test End");
        await Task.CompletedTask;
        return result;
    }

    public override DTask Clone()
    {
        var task = new DTaskTestEndCheck();
        DeepCopy(task);

        return task;
    }

    
    protected override void Clear()
    {
    }

    public void SetEndTime(int endTimeMS)
    {
        _endTime = DateTime.Now.AddMilliseconds(endTimeMS);
    }
}
