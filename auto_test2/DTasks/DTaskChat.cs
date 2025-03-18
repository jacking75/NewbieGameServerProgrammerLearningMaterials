using AutoTestClient.Dummy;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

class DTaskChat : DTask
{
    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
        _action = action;
    }

    public override async Task<DTaskResult> Run()
    {
        await Task.CompletedTask;
        var result = new DTaskResult();


        //TODO: 임시로 연결을 성공처리 한다.
        result.Ret = DTaskResultValue.Completed;
        (result.NextDTaskWaitTimeMS, result.NextDTaskIndex) = NextTask();
        Clear();

        Log.Information("Chat");
        return result;
    }


    public override DTask Clone()
    {
        var task = new DTaskChat();
        DeepCopy(task);

        return task;
    }

    protected override void Clear()
    {
    }

    
}
