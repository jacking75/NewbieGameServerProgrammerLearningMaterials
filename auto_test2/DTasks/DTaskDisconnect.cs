using AutoTestClient.Dummy;

using Serilog;


namespace AutoTestClient.DTasks;

class DTaskDisconnect : DTask
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

        //TODO: 연결 상태를 확인하고 끊어지도록 한다
        

        result.Ret = DTaskResultValue.Completed;
        (result.NextDTaskWaitTimeMS, result.NextDTask) = NextTask();

        Clear();

        Log.Information("Disconnected");
        return result;
    }

    public override DTask Clone()
    {
        var task = new DTaskDisconnect();
        DeepCopy(task);

        return task;
    }

    
    protected override void Clear()
    {
    }
}
