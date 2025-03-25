using AutoTestClient.Dummy;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

class DTaskLeaveRoom : DTask
{
    const int WaitTimeMS = 5000;


    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
        _action = action;
    }

    public override async Task<DTaskResult> Run()
    {
        Log.Information($"[LeaveRoom Try] Dummy: {_runTimeData.DummyNumber}");

        if (_alreadyActed == false)
        {
            var actionRet = await ActionRequestLeaveRoom();
            return actionRet;
        }

        var (ischk1, ret1) = CheckSuccessful();
        if (ischk1)
        {
            Log.Information($"[LeaveRoom Success] Dummy: {_runTimeData.DummyNumber}");
            return ret1;
        }

        var (_, ret2) = CheckTimeout();
        return ret2;
    }


    public override DTask Clone()
    {
        var task = new DTaskLeaveRoom();
        DeepCopy(task);
        return task;
    }

    public override void Clear()
    {
        _alreadyActed = false;
    }


    async Task<DTaskResult> ActionRequestLeaveRoom()
    {
        _endTime = DateTime.Now.AddMilliseconds(WaitTimeMS);

        var errorCode = await _action.RequestLeaveRoom();
        if (errorCode != ErrorCode.None)
        {
            Log.Error($"LeaveRoom Error. errorCode:{errorCode}");
            var result = new DTaskResult() { Ret = DTaskResultValue.Failed };
            return result;
        }

        _alreadyActed = true;

        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return ret;
    }

    (bool, DTaskResult) CheckSuccessful()
    {
        if (_runTimeData.IsLogin())
        {
            Clear();

            var result = MakeTaskResultComplete();
            return (true, result);
        }

        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return (false, ret);
    }
 


}
