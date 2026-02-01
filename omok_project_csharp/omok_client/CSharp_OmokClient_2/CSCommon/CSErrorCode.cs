using System;
using System.Collections.Generic;
using System.Text;

namespace CSCommon
{
    public enum ErrorCode
    {
        None = 0, // 에러가 아니다

        // 서버 초기화 에라
        REDIS_INIT_FAIL = 1,    // Redis 초기화 에러

        // 로그인 
        LOGIN_INVALID_AUTHTOKEN = 1001, // 로그인 실패: 잘못된 인증 토큰
        ADD_USER_DUPLICATION = 1002,
        LOGIN_INVALID_SESSION_ID = 1003,
        REMOVE_USER_SEARCH_FAILURE_USER_ID = 1004,
        USER_AUTH_SEARCH_FAILURE_USER_ID = 1005,
        USER_AUTH_ALREADY_SET_AUTH = 1006,
        LOGIN_ALREADY_WORKING = 1007,
        LOGIN_FULL_USER_COUNT = 1008,
        LOGIN_NOT_EXIST_CLIENT_SESSION = 1009,
        LOGIN_NOT_FOUND_USER = 1010,
        LOGIN_SESSION_EXPIRED = 1011,

        DB_LOGIN_INVALID_PASSWORD = 1012,
        DB_LOGIN_EMPTY_USER = 1013,
        DB_LOGIN_EXCEPTION = 1014,

        ROOM_INVALID_STATE = 1021,
        ROOM_INVALID_USER = 1022,
        ROOM_ERROR_SYSTEM = 1023,
        ROOM_INVALID_ROOM_NUMBER = 1024,
        ROOM_FAIL_ADD_USER = 1025,

        OMOK_OVERFLOW = 1031,
        OMOK_ALREADY_EXIST = 1032,
        OMOK_RENJURULE = 1033, // 쌍삼
        OMOK_TURN_NOT_MATCH = 1034,
        OMOK_NOT_STARTED = 1035,
    }
}
