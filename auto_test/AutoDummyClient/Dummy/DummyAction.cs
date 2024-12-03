using AutoTestClient.Network;

namespace AutoTestClient.Dummy
{
    public class DummyAction : DummyGame
    {
        public static Func<Int32> AllocRoomNumberFunc;

        public static Action<Int32> ReleaseRoomNumberFunc;

        public bool IsScenarioRunning { get; private set; } = false;

        public ScenarioType ScenarioType { get; private set; } = ScenarioType.NONE;

        public ScenarioResult ScenarioResult { get; private set; } = new();

        public Int32 SucceededActionCount { get; private set; } = 0;

        private string _lastAction;
        private Int32 _actionTimeoutSec;
        private Int32 _actionIntervalMilliSec;
        private DateTime _actionStartTime;
        private DateTime _scenarioStartTime;

        public void SuccessScenarioAction()
        {
            ++SucceededActionCount;
        }

        public void ScenarioPrepare(ScenarioType scenarioType)
        {
            ScenarioType = scenarioType;
            _scenarioStartTime = DateTime.Now;

            IsScenarioRunning = true;
        }

        public void ScenarioDone(bool isSucceeded, string message)
        {
            ScenarioResult.IsSucceeded = isSucceeded;
            ScenarioResult.Message = message;
            ScenarioResult.LastAction = _lastAction;
            ScenarioResult.ElapsedTimeMilliSec = DateTime.Now.Millisecond - _scenarioStartTime.Millisecond;

            IsScenarioRunning = false;
        }


        private bool _isReceivedResponse = false;

        private async Task WaitForResponse()
        {
            Monitor.IncreaseWaitForResponsePacketCount();

            while (IsScenarioRunning == true && _isReceivedResponse == false)
            {
                await Task.Delay(50);
            }

            // 누군가 true로 만들어서 깨어난 경우, 다시 false로 변경한 후 나간다.
            _isReceivedResponse = false;
        }

        public void WakeForResponse()
        {
            _isReceivedResponse = true;
        }

        public bool IsRunningActionTimeout()
        {
            // 현재 시간에서 액션 시작 시간을 뺀 값을 체크한다.
            var elapsedTimeSec = (DateTime.Now - _actionStartTime).TotalSeconds;
            if (elapsedTimeSec >= _actionTimeoutSec)
            {
                return true;
            }

            return false;
        }

        #region 연결

        public async Task<ErrorCode> ConnectAction()
        {
            ActionStart("ConnectAction");

            var result = await Connect();
            if (result != ErrorCode.None)
            {
                return result;
            }

            // 서버와 연결 완료 후 일정 시간 대기하지 않고 바로 다음 액션 (ex: 로그인)을 진행할 경우
            // 서버쪽 세션 개체가 생성되기 전에 서버 패킷 프로세서가 로그인 패킷을 먼저 처리하면서 존재하지 않는 세션에 대한 로그인을 시도할 수 있다.
            await Task.Delay(1000);

            ConnectSuccessAction();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        private void ConnectSuccessAction()
        {
            CurrnetState = DummnyState.Connected;

            Monitor.IncreaseConnectedDummyCount();
        }

        #endregion



        #region 연결 해제
        public async Task<ErrorCode> DisconnectAction()
        {
            ActionStart("DisconnectAction");

            var result = Disconnect();
            if (result != ErrorCode.None)
            {
                return result;
            }

            // 서버와 연결 해제 후 일정 시간 대기하지 않으면, 서버에서 세션 메모리가 정리되기 전에 다음 액션에 대한 패킷을 보낼 수 있다.
            await Task.Delay(1000);

            DisconnectSuccessAction();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        private void DisconnectSuccessAction()
        {
            if (CurrnetState >= DummnyState.Login)
            {
                Monitor.DecreaseLoginDummyCount();
            }

            if (CurrnetState >= DummnyState.Room)
            {
                Monitor.DecreaseRoomDummyCount();
            }

            Monitor.DecreaseConnectedDummyCount();

            CurrnetState = DummnyState.None;
        }

        #endregion



        #region 로그인

        public async Task<ErrorCode> LoginRequestAction()
        {
            ActionStart("LoginAction");

            var packet = PacketFactory.GetReqLogin(ID);
            var errorCode = await SendPacket(packet);
            if (errorCode != ErrorCode.None)
            {
                return errorCode;
            }

            // 패킷 프로세서가 서버로부터 수신 받은 로그인 응답 패킷을 처리할 때 까지 대기한다.
            await WaitForResponse();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        public void LoginSuccessAction()
        {
            CurrnetState = DummnyState.Login;

            // 로그인 완료 시 처리할 것 다 처리하고 스레드를 깨운다.
            WakeForResponse();

            Monitor.IncreaseLoginDummyCount();
        }

        #endregion



        #region 방 입장

        public async Task<ErrorCode> RoomEnterRequestAction()
        {
            ActionStart("RoomEnterAction");

            // 현재 서버는 방 입장 완료 시 요청한 방 번호를 재송신하지 않으므로
            // 방 입장 요청 전에 더미에 입장할 방 번호를 미리 세팅한 후 방 입장을 요청한다.
            EnteredRoomNumber = AllocRoomNumberFunc();

            var packet = PacketFactory.GetReqRoomEnter(EnteredRoomNumber);

            var errorCode = await SendPacket(packet);
            if (errorCode != ErrorCode.None)
            {
                // 해당 더미가 패킷 송신에 실패했을 경우, 다른 더미가 방 번호를 사용할 수 있게 다시 반환한다.
                ReleaseRoomNumberFunc(EnteredRoomNumber);
                return errorCode;
            }

            // 패킷 프로세서가 서버로부터 수신 받은 방 입장 응답 패킷을 처리할 때 까지 대기한다.
            await WaitForResponse();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        public void RoomEnterSuccessAction()
        {
            CurrnetState = DummnyState.Room;

            // 방 입장 완료 시 처리할 것 다 처리하고 스레드를 깨운다.
            WakeForResponse();

            Monitor.IncreaseRoomDummyCount();
        }

        #endregion



        #region 방 퇴장

        public async Task<ErrorCode> RoomLeaveRequestAction()
        {
            ActionStart("RoomLeaveRequestAction");

            var packet = PacketFactory.GetReqRoomLeave();

            var sendResult = await SendPacket(packet);
            if (sendResult != ErrorCode.None)
            {
                return sendResult;
            }

            await WaitForResponse();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        public void RoomLeaveSuccessAction()
        {
            var temp = EnteredRoomNumber;

            OtherUserIDList.Clear();

            EnteredRoomNumber = -1;

            CurrnetState = DummnyState.Login;

            // 방 번호 반환(이제 다른 더미가 해당 방 번호를 사용할 수 있다.)
            ReleaseRoomNumberFunc(temp);

            // 방 입장 완료 시 처리할 것 다 처리하고 스레드를 깨운다.
            WakeForResponse();

            Monitor.DecreaseRoomDummyCount();
        }

        #endregion



        #region 방 채팅

        public async Task<ErrorCode> RoomChatRequestAction()
        {
            ActionStart("RoomChatRequestAction");

            // 본인이 보낸 메시지를 서버가 정상적으로 송신했는지 확인하기 위해 저장한다.
            SendedChatMessage = $"TEST_CHAT_MESSAGE_{Number}";

            var packet = PacketFactory.GetReqRoomChat(SendedChatMessage);

            var errorCode = await SendPacket(packet);
            if (errorCode != ErrorCode.None)
            {
                return errorCode;
            }

            // 패킷 프로세서가 서버로부터 수신 받은 방 채팅 응답 패킷을 처리할 때 까지 대기한다.
            await WaitForResponse();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        public void RoomChatSuccessAction()
        {
            WakeForResponse();
        }

        #endregion



        #region 게임 시작
        public async Task<ErrorCode> GameStartRequestAction()
        {
            if (IsRunningGame == true)
            {
                return ErrorCode.None;
            }

            ActionStart("GameStartRequestAction");

            var packet = PacketFactory.GetReqGameStart();
            var errorCode = await SendPacket(packet);
            if (errorCode != ErrorCode.None)
            {
                return errorCode;
            }

            // 패킷 프로세서가 서버로부터 수신 받은 게임 시작 응답 패킷을 처리할 때 까지 대기한다.
            await WaitForResponse();

            // 방의 모든 유저가 게임 시작 요청을 통해 서버에서 실제 게임이 시작될 때 까지 대기한다.
            await WaitForAllUserGameStart();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        public void GameStartSuccessAction(bool turn, StoneType stoneType, string opponentID)
        {
            // 1. 더미 게임 정보 세팅
            SetGameInfo(turn, stoneType, opponentID);

            // 2. 더미 오목 보드 준비
            OmokBoard.Prepare();

            WakeForResponse();                  // 1
            WakeForAllUserGameStart();          // 2

            Monitor.IncreaseGameDummyCount();
        }

        #endregion



        #region 돌 놓기

        public async Task<ErrorCode> PutStoneRequestAction()
        {
            ActionStart("PutStoneAction");

            // 현재 서버에서는 돌놓기 응답 패킷에 요청자가 요청한 좌표를 재송신하지 않으므로,
            // 돌을 놓고 돌놓기를 요청한다.
            var position = GetNextPutPosition();
            OmokBoard.돌놓기(StoneType, position.X, position.Y);

            var packet = PacketFactory.GetReqPutStone(position.X, position.Y);
            var sendResult = await SendPacket(packet);
            if (sendResult != ErrorCode.None)
            {
                return sendResult;
            }

            await WaitForResponse();

            await Task.Delay(_actionIntervalMilliSec);

            return ErrorCode.None;
        }

        public void PutStoneSuccessAction(bool turn)
        {
            _isMyTurn = turn;

            WakeForResponse();
        }

        #endregion



        private void ActionStart(string actionName)
        {
            _lastAction = actionName;
            _actionStartTime = DateTime.Now;
        }

        protected void InitActionConfig(ScenarioRunnerConfig config)
        {
            _actionTimeoutSec = config.DummyActionTimeoutSec.Value;
            _actionIntervalMilliSec = config.DummyActionIntervalMilliSec.Value;
        }
    }
}

