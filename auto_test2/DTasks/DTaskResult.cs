using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

public class DTaskResult
{
    public DTaskResultValue Ret = DTaskResultValue.None;
    public Int32 NextDTaskIndex = 0;
    public Int32 NextDTaskWaitTimeMS = 0;
}

public enum DTaskResultValue
{
    None = 0,
    Continue = 1,   // 진행 중
    Completed = 2,  // 완료
    Failed = 3,     // 실패
    Terminated = 4,  // 종료. 
}
