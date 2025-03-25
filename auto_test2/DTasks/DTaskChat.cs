using AutoTestClient.Dummy;
using Serilog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.DTasks;

class DTaskChat : DTask
{
    const int WaitTimeMS = 5000;
    string _chatMessage = string.Empty;

    public override void Set(RunTimeData runTimeData, DAction action)
    {
        _runTimeData = runTimeData;
        _action = action;
    }

    public override async Task<DTaskResult> Run()
    {
        Log.Information($"[Chat Try] Dummy: {_runTimeData.DummyNumber}");

        if (_alreadyActed == false)
        {
            var actionRet = await ActionRequestChatRoom();
            return actionRet;
        }

        var (ischk1, ret1) = CheckSuccessful();
        if (ischk1)
        {
            Log.Information($"[LeaveRoom Success] Dummy: {_runTimeData.DummyNumber}");
            return ret1;
        }

        var (_, ret2) = CheckTimeout();
        return ret2;        
    }


    public override DTask Clone()
    {
        var task = new DTaskChat();
        DeepCopy(task);
        return task;
    }

    public override void Clear()
    {
    }

    async Task<DTaskResult> ActionRequestChatRoom()
    {
        _endTime = DateTime.Now.AddMilliseconds(WaitTimeMS);

        _chatMessage = GenerateChatMessage();
        _runTimeData.SetChatMessage(_chatMessage);
             
        var errorCode = await _action.RequestChatRoom(_chatMessage);
        if (errorCode != ErrorCode.None)
        {
            Log.Error($"ChatRoom Error. errorCode:{errorCode}");
            var result = new DTaskResult() { Ret = DTaskResultValue.Failed };
            return result;
        }

        _alreadyActed = true;

        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return ret;
    }
        
    string GenerateChatMessage()
    {
        int minBytes = 64;
        int maxBytes = 128;

        // 랜덤 바이트 길이 결정 (64~128 바이트 사이)
        using (var rng = RandomNumberGenerator.Create())
        {
            byte[] lengthBytes = new byte[4];
            rng.GetBytes(lengthBytes);
            int length = Math.Abs(BitConverter.ToInt32(lengthBytes, 0)) % (maxBytes - minBytes + 1) + minBytes;

            // 랜덤 바이트 생성
            byte[] randomBytes = new byte[length];
            rng.GetBytes(randomBytes);

            // Base64로 인코딩 (바이너리를 텍스트로 변환)
            // URL 안전한 Base64 형식으로 변환 ('-'와 '_'를 '+'와 '/' 대신 사용)
            string base64 = Convert.ToBase64String(randomBytes)
                .Replace('+', '-')
                .Replace('/', '_')
                .TrimEnd('='); // 패딩 제거

            return base64;
        }
    }

    (bool, DTaskResult) CheckSuccessful()
    {
        if (_runTimeData.IsChatMessageCorrect())
        {
            Clear();
            var result = MakeTaskResultComplete();
            return (true, result);
        }


        var ret = new DTaskResult() { Ret = DTaskResultValue.Continue };
        return (false, ret);
    }

}
