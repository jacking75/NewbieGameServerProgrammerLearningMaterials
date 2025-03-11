/*using AutoTestClient.Dummy;

namespace AutoTestClient.Scenario
{
    public abstract class ScenarioBase
    {
        public abstract Task Action(DummyObject dummy);

        private Int32 _maxRepeactCount;
        private Int32 _maxRunTimeSec;
        private DateTime _untilRunTime;

        public void Init(TestConfig config)
        {
            _maxRepeactCount = config.ScenarioRepeatCount.Value;
            _maxRunTimeSec = config.ScenarioRunTimeSec.Value;
            _untilRunTime = DateTime.Now.AddSeconds(_maxRunTimeSec);
        }

        protected bool IsPossbleNextAction(DummyObject dummy)
        {
            // 현재 설정 시간만큼 시나리오 진행을 완료했다.
            if (_maxRunTimeSec != 0
                && DateTime.Now >= _untilRunTime)
            {
                return false;
            }

            // 해당 더미는 설정 횟수만큼 시나리오 진행을 완료했다.
            if (_maxRepeactCount != 0
                && dummy.SucceededActionCount == _maxRepeactCount)
            {
                return false;
            }

            // 해당 더미는 다른 곳에서 강제 종료됐다.
            if (dummy.IsScenarioRunning == false)
            {
                return false;
            }

            return true;
        }

        #region Actions

        protected async Task<ErrorCode> ConnectDisconnect(DummyObject dummy)
        {
            // 접속
            var result1 = await dummy.ConnectAction();
            if (result1 != ErrorCode.None)
            {
                return result1;
            }

            // 접속 해제
            var result2 = await dummy.DisconnectAction();
            if (result2 != ErrorCode.None)
            {
                return result2;
            }

            return ErrorCode.None;
        }

        public async Task<ErrorCode> ConnectLogin(DummyObject dummy)
        {
            // 접속
            var result1 = await dummy.ConnectAction();
            if (result1 != ErrorCode.None)
            {
                return result1;
            }

            // 로그인
            var result2 = await dummy.LoginRequestAction();
            if (result2 != ErrorCode.None)
            {
                return result2;
            }

            return ErrorCode.None;
        }

        public async Task<ErrorCode> ConnectLoginDisconnect(DummyObject dummy)
        {
            // 접속 -> 로그인
            var result1 = await ConnectLogin(dummy);
            if (result1 != ErrorCode.None)
            {
                return result1;
            }

            // 접속 해제
            var result2 = await dummy.DisconnectAction();
            if (result2 != ErrorCode.None)
            {
                return result2;
            }

            return ErrorCode.None;
        }

        public async Task<ErrorCode> ConnectLoginRoomEnter(DummyObject dummy)
        {
            // 접속 -> 로그인
            var result1 = await ConnectLogin(dummy);
            if (result1 != ErrorCode.None)
            {
                return result1;
            }

            // 방 입장
            var result2 = await dummy.RoomEnterRequestAction();
            if (result2 != ErrorCode.None)
            {
                return result2;
            }

            return ErrorCode.None;
        }

        public async Task<ErrorCode> RoomEnterLeave(DummyObject dummy)
        {
            // 방 입장
            var result1 = await dummy.RoomEnterRequestAction();
            if (result1 != ErrorCode.None)
            {
                return result1;
            }

            // 방 퇴장
            var result2 = await dummy.RoomLeaveRequestAction();
            if (result2 != ErrorCode.None)
            {
                return result2;
            }

            return ErrorCode.None;
        }

        public async Task<ErrorCode> GameStart(DummyObject dummy)
        {
            // 게임 시작 요청
            var result = await dummy.GameStartRequestAction();
            if (result != ErrorCode.None)
            {
                return result;
            }

            return ErrorCode.None;
        }

        public async Task<ErrorCode> PlayGame(DummyObject dummy)
        {
            // 게임이 진행 중인 경우에만 실행.
            while (dummy.IsRunningGame == true)
            {
                // 1. 내 턴을 기다린다.
                await dummy.WaitForMyTurn();

                // 내 턴이기는 한데, 게임이 끝났다.
                if (dummy.IsRunningGame == false)
                {
                    return ErrorCode.None;
                }

                // 2. 돌 놓기 요청
                await dummy.PutStoneRequestAction();
            }

            return ErrorCode.None;
        }

        #endregion
    }
}*/