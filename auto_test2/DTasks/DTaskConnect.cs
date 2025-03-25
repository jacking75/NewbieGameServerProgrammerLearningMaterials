using AutoTestClient.Dummy;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Collections.Specialized.BitVector32;

namespace AutoTestClient.DTasks;

public class DTaskConnect : DTask
{
    string _ip;
    Int32 _port;
    

    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
        _action = action;
    }    

    public override async Task<DTaskResult> Run()
    {
        Log.Information($"[Connect Try] Dummy: {_runTimeData.DummyNumber}");


        var result = new DTaskResult();

        var errorCode = await _action.Connect(_ip, _port);
        if (errorCode != 0)
        {
            Log.Error($"Connect Error. errorCode:{errorCode}");

            result.Ret = DTaskResultValue.Failed;
            return result;
        }


        result = MakeTaskResultComplete();

        Log.Information($"[Connect Success] Dummy: {_runTimeData.DummyNumber}");
        return result;
    }


    public override DTask Clone()
    {
        var task = new DTaskConnect();
        DeepCopy(task);
                
        return task;
    }

    public override void Clear()
    {
    }
        

    public void SetConnectInfo(string ip, Int32 port)
    {
        _ip = ip;
        _port = port;
    }
}
