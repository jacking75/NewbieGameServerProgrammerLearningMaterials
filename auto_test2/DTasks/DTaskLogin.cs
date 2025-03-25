using AutoTestClient.Dummy;
using AutoTestClient.Network;
using CSCommon;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

class DTaskLogin : DTask
{
    const int WaitTimeMS = 5000;
    

    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
    }

    public override async Task<DTaskResult> Run()
    {
        Log.Information("Try - Login");
                        
        // 로그인을 하지 않았다면 로그인을 시도한다.
        if(_alreadyActed == false)
        {
            var actionRet = await ActionRequestLogin();
            return actionRet;            
        }


        // 로그인이 되었는지 확인한다.
        var (ischk1, ret1) = CheckSuccessful();
        if (ischk1)
        {
            Log.Information($"[Login Success] Dummy: {_runTimeData.DummyNumber}");
            return ret1;
        }

        // 대기 시간이 넘으면 실패로 처리한다.
        var (_, ret2) = CheckTimeout();
        return ret2;        
    }

    public override DTask Clone()
    {
        var task = new DTaskLogin();
        DeepCopy(task);

        return task;
    }


    public override void Clear()
    {
        _alreadyActed = false;
    }

    async Task<DTaskResult> ActionRequestLogin()
    {
        _endTime = DateTime.Now.AddMilliseconds(WaitTimeMS);

        var errorCode = await _action.RequestLogin(_runTimeData.UserID, _runTimeData.PassWord);
        if (errorCode != ErrorCode.None)
        {
            Log.Error($"Login Error. errorCode:{errorCode}");
            var result = new DTaskResult() { Ret = DTaskResultValue.Failed };
            return result;
        }

        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return ret;
    }

    (bool, DTaskResult) CheckSuccessful()
    {
        if (_runTimeData.IsLogin())
        {
            var result = MakeTaskResultComplete();
            return (true, result);
        }

        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return (false, ret);
    }
        

}
