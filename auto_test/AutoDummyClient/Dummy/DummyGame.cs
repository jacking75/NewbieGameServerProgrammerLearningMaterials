namespace AutoTestClient.Dummy
{
    public class DummyGame : DummyRoom
    {
        public bool IsRunningGame { get; protected set; } = false;

        public bool IsWinner { get; protected set; } = false;

        public StoneType StoneType { get; protected set; } = StoneType.Empty;

        public string OpponentID { get; protected set; } = string.Empty;

        public OmokBoard OmokBoard { get; protected set; } = new();

        protected bool _isMyTurn = false;
        protected bool _isAllUserGameStart = false;
        protected const Int32 OmokBoardWidth = 9;
        protected const Int32 OmokBoardHeight = 9;
        protected Random _random = new Random((Int32)DateTime.UtcNow.Ticks);

        public async Task WaitForAllUserGameStart()
        {
            while (_isAllUserGameStart == false)
            {
                await Task.Delay(millisecondsDelay: 50);
            }
            _isAllUserGameStart = false;
        }

        protected void WakeForAllUserGameStart()
        {
            _isAllUserGameStart = true;
        }

        public async Task WaitForMyTurn()
        {
            while (_isMyTurn == false)
            {
                await Task.Delay(100);
            }
        }

        public void WakeForMyTurn()
        {
            _isMyTurn = true;
        }

        public void GameDone(StoneType winner)
        {
            if (winner == StoneType)
            {
                IsWinner = true;
            }
            else
            {
                IsWinner = false;
            }

            CurrnetState = DummnyState.Room;

            IsRunningGame = false;
        }

        public (Int32 X, Int32 Y) GetNextPutPosition()
        {
            var empties = OmokBoard.GetEmptyPosition();

            // 더 이상 둘 곳이 없다면, 서버에서 알아서 게임 종료 패킷을 송신한다.
            // 여기서 return 하지 않고 계속 진행하면, 이미 돌이 존재하는 곳에 요청하게 된다.
            if (empties.Count == 0)
            {
                return (-1, -1);
            }

            var index = _random.Next(1, empties.Count) - 1;
            var position = empties[index];

            return (position.X, position.Y);
        }

        protected void SetGameInfo(bool turn, StoneType stoneType, string opponentID)
        {
            _isMyTurn = turn;
            StoneType = stoneType;
            OpponentID = opponentID;
            CurrnetState = DummnyState.Game;

            IsRunningGame = true;
        }

        protected void InitGameConfig()
        {
            OmokBoard.Init(OmokBoardWidth, OmokBoardHeight);
        }
    }
}

