using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

class DTaskManager
{
    List<DTask> _taskList = new();

    //TODO: TaskConfig을 사용하여 Task를 생성하도록 수정
    public bool Init(TestConfig config)
    {
        CreateTasks(config);

        LinkTasks(config.TaskConfigs);

        return true;
    }

    public List<DTask> GetTaskList()
    {
        return _taskList;
    }   


    bool CreateTasks(TestConfig config)
    {
        _taskList.Add(new DTaskConnect());
        _taskList[0].SetName("Connect", 0);

        _taskList.Add(new DTaskDisconnect());
        _taskList[1].SetName("Disconnect", 1);

        _taskList.Add(new DTaskLogin());
        _taskList[2].SetName("Login", 2);

        _taskList.Add(new DTaskEnterRoom());
        _taskList[3].SetName("EnterRoom", 3);

        _taskList.Add(new DTaskLeaveRoom());
        _taskList[4].SetName("LeaveRoom", 4);

        _taskList.Add(new DTaskChat());
        _taskList[5].SetName("Chat", 5);

        _taskList.Add(new DTaskTestEndCheck());
        _taskList[6].SetName("TestEndCheck", 6);

        return true;
    }

    bool LinkTasks(List<TaskConfig> taskConfigs)
    {
        foreach (var taskConfig in taskConfigs)
        {
            var task = GetTask(taskConfig.TaskName);
            for (int i = 0; i < taskConfig.NextTasks.Count; ++i)
            {
                var nextTask = GetTask(taskConfig.NextTasks[i]);
                task.AddNextTask(nextTask.Index, 
                                taskConfig.NextTaskProbabilityList[i], 
                                taskConfig.NextTaskWaitMinTimeMSList[i],
                                taskConfig.NextTaskWaitMaxTimeMSList[i]);
            }
        }

        return true;
    }

    DTask GetTask(string taskName)
    {
        return _taskList.Find(task => task.Name == taskName);
    }
}
