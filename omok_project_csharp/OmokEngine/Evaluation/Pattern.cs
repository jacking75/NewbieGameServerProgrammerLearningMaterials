using System;
using System.Collections.Generic;
using System.Text;

namespace OmokEngine.Evaluation;

/// <summary>
/// 패턴 정보
/// </summary>
public class Pattern
{
    public int ConsecutiveStones { get; set; }  // 연속된 돌의 개수
    public int OpenEnds { get; set; }           // 열린 끝의 개수 (0, 1, 2)
    public bool HasSpace { get; set; }          // 중간에 공간이 있는지
    public int TotalLength { get; set; }        // 전체 길이

    public Pattern(int consecutive, int openEnds, bool hasSpace = false, int totalLength = 0)
    {
        ConsecutiveStones = consecutive;
        OpenEnds = openEnds;
        HasSpace = hasSpace;
        TotalLength = totalLength;
    }
}