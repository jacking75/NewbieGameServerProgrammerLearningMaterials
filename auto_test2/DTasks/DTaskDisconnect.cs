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
        var result = new DTaskResult();

        _action.Disconnect();

        result = MakeTaskResultComplete();

        Clear();

        Log.Information("Disconnected Success");

        await Task.CompletedTask;
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
