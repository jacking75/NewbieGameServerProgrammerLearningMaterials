using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Core;

/// <summary>
/// 돌의 상태
/// </summary>
public enum Stone
{
    Empty = 0,
    Black = 1,
    White = 2
}

/// <summary>
/// 수의 유형
/// </summary>
public enum MoveType
{
    Winning,        // 이기는 수
    DefendFour,     // 4목 막기
    MakeFour,       // 4목 만들기
    DefendThree,    // 3목 막기
    MakeThree,      // 3목 만들기
    DefendTwo,      // 2목 막기
    MakeTwo,        // 2목 만들기
    Strategic,      // 전략적 수
    Neutral         // 일반 수
}