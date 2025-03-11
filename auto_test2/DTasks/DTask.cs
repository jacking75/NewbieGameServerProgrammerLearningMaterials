using AutoTestClient.Dummy;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

public abstract class DTask
{
    // 태스크를 복사할 때는 아래 필드들을 복사해야 한다.
    public string Name { get; protected set; } = string.Empty;
    protected List<int> _nextTaskProbabilityList = new();
    protected List<(int, int)> _nextTaskWaitTimeMSList = new();
    protected List<DTask> _nextTaskList = new();

    protected Random _random;

    protected RunTimeData _runTimeData;
    protected DAction _action;


    public DTask()
    {
        _random = new Random((int)DateTime.Now.Ticks);
    }
    protected void DeepCopy(DTask targetTask)
    {
        targetTask.Name = Name;
        
        //_nextTaskList.AddRange(targetTask._nextTaskList);
        targetTask._nextTaskList.AddRange(_nextTaskList);
        
        //_nextTaskProbabilityList.AddRange(targetTask._nextTaskProbabilityList);
        targetTask._nextTaskProbabilityList.AddRange(_nextTaskProbabilityList);

        //_nextTaskWaitTimeMSList.AddRange(targetTask._nextTaskWaitTimeMSList);
        targetTask._nextTaskWaitTimeMSList.AddRange(_nextTaskWaitTimeMSList);
    }


    public abstract void Set(RunTimeData runTimeData, DAction action);

    public abstract Task<DTaskResult> Run();
        
    public abstract DTask Clone();

    protected abstract void Clear();

    protected (Int32, DTask) NextTask()
    {
        /*
         0에서 99 사이의 난수를 생성합니다 (확률 총합이 100이므로).
        각 태스크의 확률을 누적해가며 계산합니다.
        누적 확률이 생성된 난수보다 크거나 같은 첫 번째 태스크를 선택합니다.

        예를 들어, _nextTaskList에 두 개의 태스크가 있고 _nextTaskProbabilityList에 [30, 70]이 들어있다면:
        - 0부터 29까지의 난수가 나오면 첫 번째 태스크 선택 (30% 확률)
        - 30부터 99까지의 난수가 나오면 두 번째 태스크 선택 (70% 확률)
         */
        
        // 0부터 99까지의 난수 생성 (_nextTaskProbabilityList 합이 100이라고 가정)
        int randomValue = _random.Next(0, 100);

        // 누적 확률 값을 계산하여 해당하는 태스크 선택
        int cumulativeProbability = 0;

        for (int i = 0; i < _nextTaskProbabilityList.Count; i++)
        {
            cumulativeProbability += _nextTaskProbabilityList[i];

            // 누적 확률이 난수보다 크거나 같으면 해당 인덱스의 태스크 선택
            if (randomValue < cumulativeProbability)
            {
                return (_nextTaskWaitTimeMSList[i].Item1, _nextTaskList[i]);
            }
        }

        // 혹시 모를 예외 상황을 위해 마지막 태스크 반환 (정상적으로는 여기까지 오지 않음)
        int lastIndex = _nextTaskList.Count - 1;
        return (_nextTaskWaitTimeMSList[lastIndex].Item1, _nextTaskList[lastIndex]);
    }

    public void SetName(string name)
    {
        Name = name;
    }

    public void AddNextTask(DTask task, int probability, int minWaitTimeMS, int maxWaitTimeMS)
    {
        _nextTaskList.Add(task);
        _nextTaskProbabilityList.Add(probability);
        _nextTaskWaitTimeMSList.Add((minWaitTimeMS, maxWaitTimeMS));
    }

 
}
