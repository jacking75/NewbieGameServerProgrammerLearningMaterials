using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.Dummy;

public class RunTimeData
{
    public Int32 DummyIndex { get; set; } = -1;
    public Int32 DummyNumber { get; set; } = -1;

    public string UserID { get; set; } = string.Empty;
    public string PassWord { get; set; } = string.Empty;

    public DummyState CurState { get; set; } = DummyState.None;

    List<(CSCommon.ErrorCode, string)> _errorList = new();

    string _sendedChatMessage = string.Empty;
    bool _isChatMessageCorrect = false;



    public void SetUserInfo(string userID, string passWord, Int32 dummyIndex, Int32 dummyNumber)
    {
        DummyIndex = dummyIndex;
        DummyNumber = dummyNumber;

        UserID = userID;
        PassWord = passWord;
    }

    public void SetState(DummyState state)
    {
        CurState = state;
    }

    public bool IsLogin()
    {
        return CurState >= DummyState.Login;
    }

    public bool IsInsideRoom()
    {
        return CurState >= DummyState.Room;
    }


    public void AddError(CSCommon.ErrorCode errorCode, string message)
    {
        _errorList.Add((errorCode, message));
    }

    public List<(CSCommon.ErrorCode, string)> GetErrorList()
    {
        return _errorList;
    }

    public bool HasError()
    {
        return _errorList.Count > 0;
    }


    public void SetChatMessage(string message)
    {
        _isChatMessageCorrect = false;
        _sendedChatMessage = message;
    }

    public void CheckChatMessage(string message)
    {
        _isChatMessageCorrect = _sendedChatMessage == message;
    }

    public bool IsChatMessageCorrect()
    {
        return _isChatMessageCorrect;
    }

}
