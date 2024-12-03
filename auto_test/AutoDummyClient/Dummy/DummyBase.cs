namespace AutoTestClient.Dummy
{
    public class DummyBase
    {
        public Int32 Index { get; protected set; } = -1;

        public Int32 Number { get; protected set; } = -1;

        public string ID { get; protected set; } = string.Empty;

        public DummnyState CurrnetState { get; protected set; } = DummnyState.None;
    }

    public enum DummnyState
    {
        None = 0,
        Connected = 1,
        Login = 2,
        Room = 3,
        Game = 4,
    }
}
