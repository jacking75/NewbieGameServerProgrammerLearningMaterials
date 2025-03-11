using AutoTestClient.Dummy;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

class DTaskLogin : DTask
{
    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
    }

    public override async Task<DTaskResult> Run()
    {
        Log.Information("Run - DTaskLogin");

        var result = new DTaskResult();
        result.Ret = DTaskResultValue.Completed;

        await Task.CompletedTask;
        return result;
    }

    public override DTask Clone()
    {
        var task = new DTaskLogin();
        DeepCopy(task);

        return task;
    }

    
    protected override void Clear()
    {
        //_isTryConnect = false;
    }
}
