namespace AutoTestClient
{
    public class ActionTimeChecker
    {
        public Action CheckingAction;

        private bool _isRunning = false;
        private Thread _updater;
        private int _updateIntervalMilliSec;

        public void Init(int updateIntervalMilliSec)
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
                CheckingAction();

                Thread.Sleep(_updateIntervalMilliSec);
            }
        }
    }
}
