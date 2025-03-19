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
    bool _isTryLogin = false;
    


    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
    }

    public override async Task<DTaskResult> Run()
    {
        Log.Information("Try - Login");

        var result = new DTaskResult();
        
        // 로그인을 하지 않았다면 로그인을 시도한다.
        if(_isTryLogin == false)
        {
            _endTime = DateTime.Now.AddMilliseconds(WaitTimeMS);

            var errorCode = await _action.RequestLogin(_runTimeData.UserID, _runTimeData.PassWord);
            if (errorCode != ErrorCode.None)
            {
                Log.Error($"Login Error. errorCode:{errorCode}");
                result.Ret = DTaskResultValue.Failed;
                return result;
            }

            result.Ret = DTaskResultValue.Continue;
            return result;
        }

        // 로그인이 되었는지 확인한다.
        if(_runTimeData.IsLoginState())
        {
            Log.Information("Login Success");

            result = MakeTaskResultComplete();
            return result;
        }   


        // 대기 시간이 넘으면 실패로 처리한다.
        if (DateTime.Now >= _endTime)
        {
            result.Ret = DTaskResultValue.Failed;
            return result;
        }


        result.Ret = DTaskResultValue.Continue;
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
        _isTryLogin = false;
    }
}
