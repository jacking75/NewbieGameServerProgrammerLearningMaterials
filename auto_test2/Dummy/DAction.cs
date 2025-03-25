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

    public Func<bool> IsConnected;


    public async Task<ErrorCode> RequestLogin(string userID, string passWord)
    {
        if(IsConnected() == false)
        {
            return ErrorCode.UnableRequestLoginNotConnected;
        }   

        var packet = PacketFactory.GetReqLogin(userID, passWord);
        var errorCode = await SendPacket(packet);
        if (errorCode != ErrorCode.None)
        {
            return errorCode;
        }

        return ErrorCode.None;
    }

    public async Task<ErrorCode> RequestEnterRoom(int roomNumber)
    {
        var packet = PacketFactory.GetReqRoomEnter(roomNumber);

        var errorCode = await SendPacket(packet);
        if (errorCode != ErrorCode.None)
        {
            return errorCode;
        }

        return ErrorCode.None;
    }

    public async Task<ErrorCode> RequestLeaveRoom()
    {
        var packet = PacketFactory.GetReqRoomLeave();

        var errorCode = await SendPacket(packet);
        if (errorCode != ErrorCode.None)
        {
            return errorCode;
        }

        return ErrorCode.None;
    }

    public async Task<ErrorCode> RequestChatRoom(string chatMessage)
    {
        var packet = PacketFactory.GetReqRoomChat(chatMessage);

        var errorCode = await SendPacket(packet);
        if (errorCode != ErrorCode.None)
        {
            return errorCode;
        }

        return ErrorCode.None;
    }


}
