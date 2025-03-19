using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.Dummy;

public class RunTimeData
{
    public string UserID { get; set; } = string.Empty;
    public string PassWord { get; set; } = string.Empty;

    public DummyState CurState { get; set; } = DummyState.None;


    public void SetUserInfo(string userID, string passWord)
    {
        UserID = userID;
        PassWord = passWord;
    }

    public void SetState(DummyState state)
    {
        CurState = state;
    }

    public bool IsLoginState()
    {
        return CurState >= DummyState.Login;
    }
}
