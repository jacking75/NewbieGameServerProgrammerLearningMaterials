namespace AutoTestClient
{
    public class ScenarioResult
    {
        public bool IsSucceeded { get; set; }

        public string Message { get; set; } = string.Empty;

        public Int32 ElapsedTimeMilliSec { get; set; }

        public string LastAction { get; set; } = string.Empty;
    }
}
