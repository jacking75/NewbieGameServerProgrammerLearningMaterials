using CSCommon;
using MessagePack;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.Versioning;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace csharp_test_client
{
    [SupportedOSPlatform("windows10.0.177630")]
    public partial class mainForm : Form
    {
        ClientSimpleTcp Network = new ClientSimpleTcp();

        bool IsNetworkThreadRunning = false;
        bool IsBackGroundProcessRunning = false;

        System.Threading.Thread NetworkReadThread = null;
        System.Threading.Thread NetworkSendThread = null;

        PacketBufferManager PacketBuffer = new PacketBufferManager();
        ConcurrentQueue<byte[]> RecvPacketQueue = new ConcurrentQueue<byte[]>();
        ConcurrentQueue<byte[]> SendPacketQueue = new ConcurrentQueue<byte[]>();

        System.Windows.Forms.Timer dispatcherUITimer = new();


        public mainForm()
        {
            InitializeComponent();
        }

        private void mainForm_Load(object sender, EventArgs e)
        {
            PacketBuffer.Init((8096 * 10), MsgPackPacketHeaderInfo.HeadSize, 2048);

            IsNetworkThreadRunning = true;
            NetworkReadThread = new System.Threading.Thread(this.NetworkReadProcess);
            NetworkReadThread.Start();
            NetworkSendThread = new System.Threading.Thread(this.NetworkSendProcess);
            NetworkSendThread.Start();

            IsBackGroundProcessRunning = true;
            dispatcherUITimer.Tick += new EventHandler(BackGroundProcess);
            dispatcherUITimer.Interval = 100;
            dispatcherUITimer.Start();

            btnDisconnect.Enabled = false;

            SetPacketHandler();
            DevLog.Write("프로그램 시작 !!!", LOG_LEVEL.INFO);
        }

        private void mainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            IsNetworkThreadRunning = false;
            IsBackGroundProcessRunning = false;

            Network.Close();
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            string address = textBoxIP.Text;

            if (checkBoxLocalHostIP.Checked)
            {
                address = "127.0.0.1";
            }

            int port = Convert.ToInt32(textBoxPort.Text);

            if (Network.Connect(address, port))
            {
                labelStatus.Text = string.Format("{0}. 서버에 접속 중", DateTime.Now);
                btnConnect.Enabled = false;
                btnDisconnect.Enabled = true;

                DevLog.Write($"서버에 접속 중", LOG_LEVEL.INFO);
            }
            else
            {
                labelStatus.Text = string.Format("{0}. 서버에 접속 실패", DateTime.Now);
            }

            PacketBuffer.Clear();
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            SetDisconnectd();
            Network.Close();
        }

        

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

                if(RecvPacketQueue.TryDequeue(out packet))
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

            labelStatus.Text = "서버 접속이 끊어짐";
        }

        void PostSendPacket(UInt16 packetID, byte[] packetData)
        {
            if (Network.IsConnected() == false)
            {
                DevLog.Write("서버 연결이 되어 있지 않습니다", LOG_LEVEL.ERROR);
                return;
            }

            var header = new MsgPackPacketHeaderInfo();
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

        void AddLobbyChat(string userID, string message)
        {
            listBoxLobbyChat.Items.Add($"[{userID}]: {message}");
        }

        void AddRoomUserList(Int64 userUniqueId, string userID)
        {
            var msg = $"{userUniqueId}: {userID}";
            listBoxRoomUserList.Items.Add(msg);
        }

        void RemoveRoomUserList(Int64 userUniqueId)
        {
            object removeItem = null;

            foreach( var user in listBoxRoomUserList.Items)
            {
                var items = user.ToString().Split(":");
                if( items[0].ToInt64() == userUniqueId)
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


        // 로그인 요청
        private void button2_Click(object sender, EventArgs e)
        {
            var loginReq = new PKTReqLogin();
            loginReq.UserID = textBoxUserID.Text;
            loginReq.AuthToken = textBoxUserPW.Text;
            var packet = MessagePackSerializer.Serialize(loginReq);
                        
            PostSendPacket(PacketID.ReqLogin, packet);            
            DevLog.Write($"로그인 요청:  {textBoxUserID.Text}, {textBoxUserPW.Text}");
        }

        private void btn_RoomEnter_Click(object sender, EventArgs e)
        {
            //var requestPkt = new RoomEnterReqPacket();
            //requestPkt.SetValue(textBoxRoomNumber.Text.ToInt32());

            //PostSendPacket(PACKET_ID.ROOM_ENTER_REQ, requestPkt.ToBytes());
            DevLog.Write($"방 입장 요청:  {textBoxRoomNumber.Text} 번");
        }

        private void btn_RoomLeave_Click(object sender, EventArgs e)
        {
            //PostSendPacket(PACKET_ID.ROOM_LEAVE_REQ,  null);
            DevLog.Write($"방 입장 요청:  {textBoxRoomNumber.Text} 번");
        }

        private void btnRoomChat_Click(object sender, EventArgs e)
        {
            if(textBoxRoomSendMsg.Text.IsEmpty())
            {
                MessageBox.Show("채팅 메시지를 입력하세요");
                return;
            }

            //var requestPkt = new RoomChatReqPacket();
            //requestPkt.SetValue(textBoxRoomSendMsg.Text);

            //PostSendPacket(PACKET_ID.ROOM_CHAT_REQ, requestPkt.ToBytes());
            DevLog.Write($"방 채팅 요청");
        }

        private void btnMatching_Click(object sender, EventArgs e)
        {
            //PostSendPacket(PACKET_ID.MATCH_USER_REQ, null);
            DevLog.Write($"매칭 요청");
        }

        // 로비 리스트 요청
        private void button3_Click(object sender, EventArgs e)
        {
            //PostSendPacket(PACKET_ID.LOBBY_LIST_REQ, null);
            DevLog.Write($"방 릴레이 요청");
        }

        // 로비 입장 요청
        private void button4_Click(object sender, EventArgs e)
        {
            var request = new PKTReqLobbyEnter();
            request.LobbyNumber = textBox1.Text.ToInt16();
            var packet = MessagePackSerializer.Serialize(request);

            PostSendPacket(PacketID.ReqLobbyEnter, packet);
            DevLog.Write($"로비 들어가기 요청. 번호: {textBox1.Text}");
        }

        // 로비 나가기 요청
        private void button5_Click(object sender, EventArgs e)
        {
            PostSendPacket(PacketID.ReqLobbyLeave, null);
            DevLog.Write($"로비 나가기 요청. 번호: {textBox1.Text}");
        }

        private void listBoxRoomChatMsg_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void textBoxRelay_TextChanged(object sender, EventArgs e)
        {

        }

        private void btn_PutStoneClick(object sender, EventArgs e)
        {
            //var requestPkt = new PutStoneReqPacket();
            //requestPkt.SetValue(xPosTextNumber.Text.ToInt16(), yPosTextNumber.Text.ToInt16());

            //PostSendPacket(PACKET_ID.PUT_STONE_REQ, requestPkt.ToBytes());
            DevLog.Write($"put stone 요청 : x  [  {xPosTextNumber.Text}  ], y : [ {yPosTextNumber.Text} ] ");

        }

        private void btn_GameStartClick(object sender, EventArgs e)
        {
            //PostSendPacket(PACKET_ID.GAME_START_REQ, null);
            DevLog.Write($"게임시작 요청");
        }

        // 로비 채팅
        private void button1_Click(object sender, EventArgs e)
        {
            var request = new PKTReqLobbyChat();
            request.Message = textBoxLobbyChat.Text;
            var packet = MessagePackSerializer.Serialize(request);

            PostSendPacket(PacketID.ReqLobbyChat, packet);
            DevLog.Write("로비 채팅 요청");
        }
    }
}
