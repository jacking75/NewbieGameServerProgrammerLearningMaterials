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
        Log.Information($"[DisConnect Try] Dummy: {_runTimeData.DummyNumber}");


        var result = new DTaskResult();

        _action.Disconnect();

        result = MakeTaskResultComplete();

        Clear();


        Log.Information($"[DisConnect Success] Dummy: {_runTimeData.DummyNumber}");
        await Task.CompletedTask;
        return result;
    }

    public override DTask Clone()
    {
        var task = new DTaskDisconnect();
        DeepCopy(task);

        return task;
    }


    public override void Clear()
    {
    }
}
