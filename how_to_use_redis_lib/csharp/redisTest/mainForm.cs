using CloudStructures;
using CloudStructures.Structures;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace csharp_test_client
{
    public partial class mainForm : Form
    {
        bool IsBackGroundProcessRunning = false;
        
        System.Windows.Forms.Timer dispatcherUITimer = new();

        
        public mainForm()
        {
            InitializeComponent();
        }

        private void mainForm_Load(object sender, EventArgs e)
        {            
            IsBackGroundProcessRunning = true;
            dispatcherUITimer.Tick += new EventHandler(BackGroundProcess);
            dispatcherUITimer.Interval = 100;
            dispatcherUITimer.Start();

        
            DevLog.Write("프로그램 시작 !!!", LOG_LEVEL.INFO);
        }

        private void mainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            IsBackGroundProcessRunning = false;

        }
        
        void BackGroundProcess(object sender, EventArgs e)
        {
            ProcessLog();
                        
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
     
        RedisConnection GetRedisConnection(string address)
        {   
            var config = new RedisConfig("test", address);
            var Connection = new RedisConnection(config);
            return Connection;
        }

        async Task<List<string>> MultiLPopTest(string address, string key)
        {
            var readDataList = new List<string>();

            var redis = new RedisList<string>(GetRedisConnection(address), key, null);
            
            while (true)
            {
                var result = await redis.LeftPopAsync();

                if (result.HasValue)
                {
                    readDataList.Add(result.Value);
                }
                else 
                {
                    break;
                }
            }

            return readDataList;           
        }

        
        // 복수의 LPop 테스트 시작
        private async void button3_Click(object sender, EventArgs e)
        {
            DevLog.Write("Multi Session LPop Test - start", LOG_LEVEL.INFO);

            var key = "MLPoptest";
            var redis = new RedisList<string>(GetRedisConnection(textBox1.Text), key, null);

            for(var i = 0; i < 100; ++i)
            {
                await redis.RightPushAsync($"abcde_{i}");
            }


            var task1 = Task.Run(() => MultiLPopTest(textBox1.Text, key));
            var task2 = Task.Run(() => MultiLPopTest(textBox1.Text, key));
            var task3 = Task.Run(() => MultiLPopTest(textBox1.Text, key));

            Task.WaitAll(task1, task2, task3);

            
            listBox1.Items.Clear();
            listBox2.Items.Clear();
            listBox3.Items.Clear();

            DevLog.Write($"Multi Session LPop Test - index 1 - Count:{task1.Result.Count}", LOG_LEVEL.INFO);
            foreach (var value in task1.Result)
            {
                listBox1.Items.Add(value);
            }
            DevLog.Write($"Multi Session LPop Test - index 2 - Count:{task2.Result.Count}", LOG_LEVEL.INFO);
            foreach (var value in task2.Result)
            {
                listBox2.Items.Add(value);
            }
            DevLog.Write($"Multi Session LPop Test - index 3 - Count:{task3.Result.Count}", LOG_LEVEL.INFO);
            foreach (var value in task3.Result)
            {
                listBox3.Items.Add(value);
            }

            DevLog.Write("Multi Session LPop Test - end", LOG_LEVEL.INFO);
        }

    }

    
}
