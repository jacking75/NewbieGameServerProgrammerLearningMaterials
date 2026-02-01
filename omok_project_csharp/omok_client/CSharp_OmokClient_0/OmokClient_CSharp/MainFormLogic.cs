using MessagePack;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace OmokClient
{
    public partial class MainForm : Form
    {
        ClientSceen CurSceen = ClientSceen.NONE;

        ClientSimpleTcp Network = new ClientSimpleTcp();

        bool IsNetworkThreadRunning = false;
        bool IsBackGroundProcessRunning = false;

        System.Threading.Thread NetworkReadThread = null;
        System.Threading.Thread NetworkSendThread = null;

        PacketBufferManager PacketBuffer = new PacketBufferManager();
        ConcurrentQueue<byte[]> RecvPacketQueue = new ConcurrentQueue<byte[]>();
        ConcurrentQueue<byte[]> SendPacketQueue = new ConcurrentQueue<byte[]>();

        System.Windows.Forms.Timer dispatcherUITimer = new();


        

        void NetworkReadProcess()
        {
            while (IsNetworkThreadRunning)
            {
                if (Network.IsConnected() == false)
                {
                    System.Threading.Thread.Sleep(1);
                    continue;
                }

                var recvData = Network.Receive();

                if (recvData != null)
                {
                    PacketBuffer.Write(recvData.Item2, 0, recvData.Item1);

                    while (true)
                    {
                        var data = PacketBuffer.Read();
                        if (data == null)
                        {
                            break;
                        }

                        RecvPacketQueue.Enqueue(data);
                    }
                    //DevLog.Write($"받은 데이터: {recvData.Item2}", LOG_LEVEL.INFO);
                }
                else
                {
                    Network.Close();
                    SetDisconnectd();
                    DevLog.Write("서버와 접속 종료 !!!", LOG_LEVEL.INFO);
                }
            }
        }

        void NetworkSendProcess()
        {
            while (IsNetworkThreadRunning)
            {
                System.Threading.Thread.Sleep(1);

                if (Network.IsConnected() == false)
                {
                    continue;
                }


                if (SendPacketQueue.TryDequeue(out var packet))
                {
                    Network.Send(packet);
                }
            }
        }


        void BackGroundProcess(object sender, EventArgs e)
        {
            ProcessLog();

            try
            {
                byte[] packet = null;

                if (RecvPacketQueue.TryDequeue(out packet))
                {
                    PacketProcess(packet);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("BackGroundProcess. error:{0}", ex.Message));
            }
        }

        private void ProcessLog()
        {
            // 너무 이 작업만 할 수 없으므로 일정 작업 이상을 하면 일단 패스한다.
            int logWorkCount = 0;

            while (IsBackGroundProcessRunning)
            {
                System.Threading.Thread.Sleep(1);

                string msg;

                if (DevLog.GetLog(out msg))
                {
                    ++logWorkCount;

                    if (listBoxLog.Items.Count > 512)
                    {
                        listBoxLog.Items.Clear();
                    }

                    listBoxLog.Items.Add(msg);
                    listBoxLog.SelectedIndex = listBoxLog.Items.Count - 1;
                }
                else
                {
                    break;
                }

                if (logWorkCount > 8)
                {
                    break;
                }
            }
        }


        public void SetDisconnectd()
        {
            if (btnConnect.Enabled == false)
            {
                btnConnect.Enabled = true;
                btnDisconnect.Enabled = false;
            }

            while (true)
            {
                if (SendPacketQueue.TryDequeue(out var temp) == false)
                {
                    break;
                }
            }

            listBoxRoomChatMsg.Items.Clear();
            listBoxRoomUserList.Items.Clear();

            EndGame();

            labelStatus.Text = "서버 접속이 끊어짐";
        }
                

        void AddRoomUserList(string userID)
        {
            listBoxRoomUserList.Items.Add(userID);
        }

        void RemoveRoomUserList(string userID)
        {
            object removeItem = null;

            foreach (var user in listBoxRoomUserList.Items)
            {
                if ((string)user == userID)
                {
                    removeItem = user;
                    return;
                }
            }

            if (removeItem != null)
            {
                listBoxRoomUserList.Items.Remove(removeItem);
            }
        }

        string GetOtherPlayer(string myName)
        {
            if (listBoxRoomUserList.Items.Count != 2)
            {
                return null;
            }

            var firstName = (string)listBoxRoomUserList.Items[0];
            if (firstName == myName)
            {
                return firstName;
            }
            else
            {
                return (string)listBoxRoomUserList.Items[1];
            }
        }




        void PostSendPacket(UInt16 packetID, byte[] packetData)
        {
            if (Network.IsConnected() == false)
            {
                DevLog.Write("서버 연결이 되어 있지 않습니다", LOG_LEVEL.ERROR);
                return;
            }

            var header = new CSCommon.MsgPackPacketHeaderInfo();
            header.ID = packetID;
            header.Type = 0;

            if (packetData != null)
            {
                header.TotalSize = (UInt16)packetData.Length;

                header.Write(packetData);
            }
            else
            {
                packetData = header.Write();
            }

            SendPacketQueue.Enqueue(packetData);
        }

        void SendPacketOmokPut(int x, int y)
        {
            var requestPkt = new CSCommon.PKTReqPutMok
            {
                PosX = x,
                PosY = y
            };

            var packet = MessagePackSerializer.Serialize(requestPkt);
            PostSendPacket(CSCommon.PacketID.ReqPutMok, packet);

            DevLog.Write($"put stone 요청 : x  [ {x} ], y: [ {y} ] ");
        }
    }

    enum ClientSceen
    {
        NONE = 0,
        LOGIN = 1,
        ROOM = 2,
        GAME_READY = 3,
        GAME_PLAYING = 4,
        GAME_END = 5,
    }
}
