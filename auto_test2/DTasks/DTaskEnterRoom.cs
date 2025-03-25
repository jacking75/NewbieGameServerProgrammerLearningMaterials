using AutoTestClient.Dummy;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

class DTaskEnterRoom : DTask
{
    const int WaitTimeMS = 5000;


    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
        _action = action;
    }

    public override async Task<DTaskResult> Run()
    {
        Log.Information($"[EnterRoom Try] Dummy: {_runTimeData.DummyNumber}");

        if (_alreadyActed == false)
        {
            var actionRet = await ActionRequestEnterRoom();
            return actionRet;
        }
        
        var (ischk1, ret1) = CheckSuccessful();
        if (ischk1)
        {
            Log.Information($"[EnterRoom Success] Dummy: {_runTimeData.DummyNumber}");
            return ret1;
        }


        // 대기 시간이 넘으면 실패로 처리한다.
        var (_, ret2) = CheckTimeout();
        return ret2;
    }


    public override DTask Clone()
    {
        var task = new DTaskEnterRoom();
        DeepCopy(task);
        return task;
    }

    public override void Clear() 
    {
        _alreadyActed = false;
    }


    async Task<DTaskResult> ActionRequestEnterRoom()
    {
        _endTime = DateTime.Now.AddMilliseconds(WaitTimeMS);

        var roomNumber = RoomNumberAllocator.Alloc();
        var errorCode = await _action.RequestEnterRoom(roomNumber);
        if (errorCode != ErrorCode.None)
        {
            RoomNumberAllocator.Release(roomNumber);

            Log.Error($"EnterRoom Error. errorCode:{errorCode}");
            var result = new DTaskResult() { Ret = DTaskResultValue.Failed };
            return result;
        }

        _alreadyActed = true;

        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return ret;
    }

    (bool, DTaskResult) CheckSuccessful()
    {
        if (_runTimeData.IsInsideRoom())
        {
            Log.Information("Login Success");

            Clear();

            var result = MakeTaskResultComplete();
            return (true, result);
        }

        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return (false, ret);
    }

    /*bool IsEnterRoomSuccessful()
    {
        return _runTimeData.IsInsideRoom() ? true : false;
    }*/


}
