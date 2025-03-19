using AutoTestClient.DTasks;
using AutoTestClient.Network;
using Serilog;


namespace AutoTestClient.Dummy;

public class DummyObject
{
    public Int32 Index { get; protected set; } = -1;

    public Int32 Number { get; protected set; } = -1;

    public string ID { get; protected set; } = string.Empty;

    DNetwork _dNetwork = new();

    PacketProcessor _packetProcessor = new();

    RunTimeData _runTimeData = new();
    DAction _action = new();
    
    List<DTask> _taskList = new();

    //public DummnyState CurrnetState { get; protected set; } = DummnyState.None;


    public void Init(Int32 index, Int32 number, TestConfig config, List<DTask> taskList)
    {
        Index = index;
        Number = number;
        ID = $"DUMMY_{number}";


        _dNetwork.InitNetworkConfig(config);
        _dNetwork.AddPacketToPacketProcessorFunc = _packetProcessor.AddPacket;

        _packetProcessor.Init(this);

        //TODO: 행동을 붙인다
        _action.Connect = _dNetwork.Connect;
        _action.Disconnect = _dNetwork.Disconnect;
        _action.SendPacket = _dNetwork.SendPacket;


        
        foreach (var task in taskList)
        {
            var newTask = task.Clone();
            
            newTask.Set(_runTimeData, _action);

            if(newTask.Name == "TestEndCheck")
            {
                var testEndCheckTask = (DTaskTestEndCheck)newTask;
                testEndCheckTask.SetEndTime(config.TestRunTimeMS); 
            } else if(newTask.Name == "Connect")
            {
                var (ip, port) = config.stringToIPPort();
                
                var connectTask = (DTaskConnect)newTask;
                connectTask.SetConnectInfo(ip, port);
            }

            _taskList.Add(newTask);
        }


        SetRuntimeData();
    }

    void SetRuntimeData()
    {
        _runTimeData.SetUserInfo(ID, "TEST_TOKEN");
    }


    public async Task<DResult> Run()
    {
        var curTask = _taskList[0];

        while (true)
        {
            _dNetwork.ReceiveAndAddPacketToPacketProcessor();

            _packetProcessor.Update();

            var taskResult = await curTask.Run();
            
            if (taskResult.Ret == DTaskResultValue.Completed)
            {
                curTask = _taskList[taskResult.NextDTaskIndex];

                Log.Information($"[[Next Task]] Duumy Number: {Number}, TaskName:{curTask.Name}");
                await Task.Delay(taskResult.NextDTaskWaitTimeMS);
            }
            else if (taskResult.Ret == DTaskResultValue.Failed)
            {
                Log.Error($"[[Failed Task]] Duumy Number: {Number}");
                break;
            } else if(taskResult.Ret == DTaskResultValue.Terminated)
            {
                Log.Information($"[[Terminate Task]] Duumy Number: {Number}");
                break;
            }

            //클라이언트가 60프레임으로 동작하는 것을 가정해서 1프레임에 해당하는 시간을 기다린다.
            await Task.Delay(16);
        }

        var result = new DResult();

        return result;
    }
}
