/*
namespace AutoTestClient.Network
{
    public class Receiver
    {
        public Action ReceiveAction;

        private bool _isRunning = false;
        private Thread _updater;
        private Int32 _updateIntervalMilliSec;

        public void Init(Int32 updateIntervalMilliSec)
        {
            _updateIntervalMilliSec = updateIntervalMilliSec;
            _updater = new Thread(Work);
        }

        public void Start()
        {
            _isRunning = true;
            _updater.Start();
        }

        public void Stop()
        {
            _isRunning = false;
            _updater.Join();
        }

        private void Work()
        {
            while (_isRunning == true)
            {
                ReceiveAction();

                Thread.Sleep(_updateIntervalMilliSec);
            }
        }
    }
}
*/