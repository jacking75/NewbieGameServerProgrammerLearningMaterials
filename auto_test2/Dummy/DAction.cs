using AutoTestClient.Network;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Collections.Specialized.BitVector32;

namespace AutoTestClient.Dummy;

public class DAction
{
    public Func<string, Int32, Task<ErrorCode>> Connect;
    public Func<ErrorCode> Disconnect;
    public Func<byte[], Task<ErrorCode>> SendPacket;


    public async Task<ErrorCode> RequestLogin(string userID, string passWord)
    {
        var packet = PacketFactory.GetReqLogin(userID, passWord);
        var errorCode = await SendPacket(packet);
        if (errorCode != ErrorCode.None)
        {
            return errorCode;
        }

        return ErrorCode.None;
    }
}
