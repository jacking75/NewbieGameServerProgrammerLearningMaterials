using AutoTestClient.Dummy;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

public class DTaskConnect : DTask
{
    string _ip;
    Int32 _port;
    bool _isTryConnect = false;

    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
        _action = action;
    }    

    public override async Task<DTaskResult> Run()
    {
        await Task.CompletedTask;
        var result = new DTaskResult();

        if (_isTryConnect == false)
        {
            _isTryConnect = true;

            Log.Information("Try Connected");

            result.Ret = DTaskResultValue.Continue;
            return result;
        }


        //TODO: 임시로 연결을 성공처리 한다.
        result.Ret = DTaskResultValue.Completed;
        (result.NextDTaskWaitTimeMS, result.NextDTask) = NextTask();
        Clear();

        Log.Information("Connected");
        return result;
    }


    public override DTask Clone()
    {
        var task = new DTaskConnect();
        DeepCopy(task);
                
        return task;
    }

    protected override void Clear()
    {
        _isTryConnect = false;
    }
        

    public void SetConnectInfo(string ip, Int32 port)
    {
        _ip = ip;
        _port = port;
    }
}
