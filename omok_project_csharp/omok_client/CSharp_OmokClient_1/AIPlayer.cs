using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace csharp_test_client
{
    class AI
    {
        enum POINT //****오리지널 점수표!!! 각 상황에따른 점수판의 점수 정의
        {
            P_GOOD0 = 1,                        //돌이 없는칸
            P_BADb1 = 1250, P_BAD1 = 5000,  //한쪽이 막혀있는 적의돌, 양쪽이 뚤려있는 적의돌
            P_GOODb1 = 1250, P_GOOD1 = 5000,    //한쪽이 막혀있는 나의돌, 양쪽이 뚤려있는 나의돌
            P_BADb2 = 2500, P_BAD2 = 12000, //한쪽이 막혀있는 2개의연속된 적의돌, 양쪽이 뚤려있는 2개의연속된 적의돌
            P_GOODb2 = 2500, P_GOOD2 = 12000,   //한쪽이 막혀있는 2개의연속된 나의돌, 양쪽이 뚤려있는 2개의연속된 나의돌
            P_BADb3 = 11000, P_BAD3 = 60000,    //한쪽이 막혀있는 3개의연속된 적의돌, 양쪽이 뚤려있는 3개의연속된 적의돌
            P_GOODb3 = 11000, P_GOOD3 = 130000, //한쪽이 막혀있는 3개의연속된 나의돌, 양쪽이 뚤려있는 3개의연속된 나의돌
            P_BADb4 = 200000, P_BAD4 = 200000,  //한쪽이 막혀있는 4개의연속된 적의돌, 양쪽이 뚤려있는 4개의연속된 적의돌
            P_GOOD4 = 99999999              //4개연속인 나의 돌
        };

        /*/ 개선 (공격적으로 변경)
        enum POINT // 각 상황에따른 점수판의 점수 정의
        {
            P_GOOD0=1,						//돌이 없는칸
            P_BADb1=500,	P_BAD1=2000,	//한쪽이 막혀있는 적의돌, 양쪽이 뚤려있는 적의돌
            P_GOODb1=1250,	P_GOOD1=5000,	//한쪽이 막혀있는 나의돌, 양쪽이 뚤려있는 나의돌
            P_BADb2=1000,	P_BAD2=4000,	//한쪽이 막혀있는 2개의연속된 적의돌, 양쪽이 뚤려있는 2개의연속된 적의돌
            P_GOODb2=2500,	P_GOOD2=12000,	//한쪽이 막혀있는 2개의연속된 나의돌, 양쪽이 뚤려있는 2개의연속된 나의돌
            P_BADb3=11000,	P_BAD3=60000,	//한쪽이 막혀있는 3개의연속된 적의돌, 양쪽이 뚤려있는 3개의연속된 적의돌
            P_GOODb3=11000,	P_GOOD3=130000,	//한쪽이 막혀있는 3개의연속된 나의돌, 양쪽이 뚤려있는 3개의연속된 나의돌
            P_BADb4=200000,	P_BAD4=200000,	//한쪽이 막혀있는 4개의연속된 적의돌, 양쪽이 뚤려있는 4개의연속된 적의돌
            P_GOOD4=99999999				//4개연속인 나의 돌
        };//*/

        enum DIR { DIR_LR, DIR_LTRB, DIR_TB, DIR_RTLB };
        enum PLAYERFLAG { EMPTY, PLAYER1, PLAYER2 };

        int[,] CanWinBoard; // 이길가능성 있는곳 체크 보드
        int[,] Board; // 바둑판
        int[,] PointBoard; // 점수보드

        public AI() // 생성자
        {
            PointBoard = new int[19, 19];
            CanWinBoard = new int[19, 19];
        }

        public AI(int[,] board)
        {
            PointBoard = new int[19, 19];
            CanWinBoard = new int[19, 19];

            Board = board;
        }

        public void PUT_POINT(int x, int y, int p)
        {
            PointBoard[x, y] += p;
        }

        public int GET_POINT(int x, int y)
        {
            return PointBoard[x, y];
        }

        public int GET_CANWIN(int x, int y)
        {
            return CanWinBoard[x, y];
        }

        public void initBoard(int[,] board)
        {
            for (int i = 0; i < 19; i++)
            {
                for (int j = 0; j < 19; j++)
                {
                    board[i, j] = 0;
                }
            }
        }

        public int CheckBoard(int X, int Y)  //보드와 보드의 가로세로 길이 특정위치를 입력하면 그부분의 보드 상태를 읽어서 리턴한다
        {
            if (X >= 19 || Y >= 19 || X < 0 || Y < 0) //보드의 범위를 벗어나면 -1을 리턴
                return -1;

            return Board[X, Y]; //보드의 상태를 읽어온다
        }


        public void CheckCanWinBoard(int Enemyflag, int Direction) // 놓아서 이길가능성이 있는곳을 검색해서 보드 만듬
        {
            int i = 0, j = 0;
            int XOffset, YOffset;
            int CheckRet;

            initBoard(CanWinBoard); // 보드 초기화

            for (j = 0; j < 19; ++j)
                for (i = 0; i < 19; ++i)
                {
                    if (CheckBoard(i, j) != Enemyflag) // i,j위치에 있는 돌이 적의 돌이 아닌가?
                    {
                        switch (Direction) // 아니라면 검색 방향에따라 검색시작
                        {
                            case (int)DIR.DIR_LR:
                                // - 체크
                                CheckRet = CheckBoard(i - 1, j);
                                if (CheckRet > 0 && CheckRet != Enemyflag)
                                    break;

                                for (XOffset = 0, YOffset = 0; XOffset < 5; ++XOffset)  // 보드밖으로 나가거나 적의 돌을 만날때까지 오프셋 이동
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet == -1)
                                        break;
                                }

                                if (XOffset == 5)
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet <= 0)
                                        for (XOffset = 0, YOffset = 0; XOffset < 5; ++XOffset)
                                            if (CheckBoard(i + XOffset, j + YOffset) == 0)
                                                CanWinBoard[i + XOffset, j + YOffset] = 1;
                                }

                                break;

                            case (int)DIR.DIR_LTRB:
                                // ＼ 체크
                                CheckRet = CheckBoard(i - 1, j - 1);
                                if (CheckRet > 0 && CheckRet != Enemyflag)
                                    break;

                                for (XOffset = 0, YOffset = 0; XOffset < 5; ++XOffset, ++YOffset) // 보드밖으로 나가거나 적의 돌을 만날때까지 오프셋 이동
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet == -1)
                                        break;
                                }

                                if (XOffset == 5)
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet <= 0)
                                        for (XOffset = 0, YOffset = 0; XOffset < 5; ++XOffset, ++YOffset)
                                            if (CheckBoard(i + XOffset, j + YOffset) == 0)
                                                CanWinBoard[i + XOffset, j + YOffset] = 1;
                                }

                                break;

                            case (int)DIR.DIR_TB:
                                // | 체크
                                CheckRet = CheckBoard(i, j - 1);
                                if (CheckRet > 0 && CheckRet != Enemyflag)
                                    break;

                                for (XOffset = 0, YOffset = 0; YOffset < 5; ++YOffset) // 보드밖으로 나가거나 적의 돌을 만날때까지 오프셋 이동
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet == -1)
                                        break;
                                }

                                if (YOffset == 5)
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet <= 0)
                                        for (XOffset = 0, YOffset = 0; YOffset < 5; ++YOffset)
                                            if (CheckBoard(i + XOffset, j + YOffset) == 0)
                                                CanWinBoard[i + XOffset, j + YOffset] = 1;
                                }

                                break;

                            case (int)DIR.DIR_RTLB:
                                // / 체크
                                CheckRet = CheckBoard(i + 1, j - 1);
                                if (CheckRet > 0 && CheckRet != Enemyflag)
                                    break;

                                for (XOffset = 0, YOffset = 0; YOffset < 5; --XOffset, ++YOffset) // 보드밖으로 나가거나 적의 돌을 만날때까지 오프셋 이동
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet == -1)
                                        break;
                                }

                                if (YOffset == 5)
                                {
                                    CheckRet = CheckBoard(i + XOffset, j + YOffset);
                                    if (CheckRet == Enemyflag || CheckRet <= 0)
                                        for (XOffset = 0, YOffset = 0; YOffset < 5; --XOffset, ++YOffset)
                                            if (CheckBoard(i + XOffset, j + YOffset) == 0)
                                                CanWinBoard[i + XOffset, j + YOffset] = 1;
                                }

                                break;
                        }
                    }
                }
        }





        public void AI_GetMaxPointRandom(ref int CursorX, ref int CursorY) // AI점수판에서 가장 높은 점수중 하나를 랜덤으로 선택
        {
            int MaxPoint, AmoutOfMaxPoint = 0;
            int i, j;
            int NthPlace;
            int Count = 0;
            Random rand = new Random();
            MaxPoint = PointBoard[0, 0];

            for (i = 0; i < 19; i++)
            {// 점수판에서 가장 높은 점수를 찾음
                for (j = 0; j < 19; j++)
                {
                    if (MaxPoint < PointBoard[i, j])
                        MaxPoint = PointBoard[i, j];
                }
            }
            for (i = 0; i < 19; i++)
            {// 점수판에 위에서 찾은 높은 점수가 몇개 있는지 카운트
                for (j = 0; j < 19; j++)
                {
                    if (MaxPoint == PointBoard[i, j])
                        ++AmoutOfMaxPoint;
                }
            }

            NthPlace = rand.Next() % AmoutOfMaxPoint; // N개중에서 하나를 선택

            for (i = 0; i < 19; i++)
            {
                for (j = 0; j < 19; j++)
                {
                    if (MaxPoint == PointBoard[i, j]) //N번째 높은숫자에 커서를 위치시킴
                        if (Count++ == NthPlace)
                        {
                            CursorX = i;
                            CursorY = j;
                            break;
                        }
                }
            }

        }


        public void AI_PutAIPlayer(ref int CursorX, ref int CursorY, bool OverOmok, int AIPlayerNumber) // AI플레이어 제어 함수
        {
            int i, j, k = 0; // 임시변수
            int EOLX1, EOLX2, EOLY1, EOLY2; // 현재위치로부터 한 방향의 라인을 처리할때 양 끝부분 바깥쪽의 좌표
            int XOffset, YOffset; // 오프셋 변수
            int Player; // 적의 턴 번호

            if (AIPlayerNumber == 1) // AI플레이어의 번호에따라 적의 번호도 달라진다
                Player = 2;
            else
                Player = 1;

            initBoard(PointBoard);
            initBoard(CanWinBoard);

            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) // 돌이 하나도 없으면 그냥 중앙에 놓는다
                {
                    if (CheckBoard(i, j) == 1 || CheckBoard(i, j) == 2)
                    {
                        k++;
                        break;
                    }
                }
            }

            if (k == 0)
            {
                CursorX = 19 / 2;
                CursorY = 19 / 2;
                return;
            }


            CheckCanWinBoard(Player, (int)DIR.DIR_LR); // 이길 가능성이 이있는곳을 체크
            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset + 1, j + YOffset) == AIPlayerNumber; ++XOffset) ;//가로방향중 오른쪽으로 이동하면서 빈공간또는 나의돌의개수를 체크
                    EOLX1 = i + XOffset + 1;

                    EOLY1 = j + YOffset;
                    k = XOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset - 1, j - YOffset) == AIPlayerNumber; ++XOffset) ;//가로방향 위와 반대방향으로 체크
                    EOLX2 = i - XOffset - 1;

                    EOLY2 = j - YOffset;
                    XOffset += k; //합계를 구한다

                    if ((XOffset < 5 || (XOffset >= 5 && OverOmok)) && ((GET_CANWIN(i, j) == 0 ? false : true) || XOffset > 3)) // 돌이 4개이상이거나 이길가능성이 있는위치에 육목이 불가할때는 5보다 적은개수만, 육목가능할땐 커도 작아도 상관없다
                    {
                        switch (XOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0) //라인의 양쪽이 빈공간인가?
                                    PUT_POINT(i, j, (int)POINT.P_GOOD1);

                                else // 아닌가
                                    PUT_POINT(i, j, (int)POINT.P_GOODb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb3);

                                break;

                            case 4:
                                PUT_POINT(i, j, (int)POINT.P_GOOD4);
                                break;
                        }
                    }
                }
            }


            CheckCanWinBoard(Player, (int)DIR.DIR_LTRB); //위 작업을 대각선으로 반복
            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset + 1, j + YOffset + 1) == AIPlayerNumber; ++XOffset, ++YOffset) ;
                    EOLX1 = i + XOffset + 1;

                    EOLY1 = j + YOffset + 1;
                    k = XOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset - 1, j - YOffset - 1) == AIPlayerNumber; ++XOffset, ++YOffset) ;
                    EOLX2 = i - XOffset - 1;

                    EOLY2 = j - YOffset - 1;
                    XOffset += k;

                    if ((XOffset < 5 || (XOffset >= 5 && OverOmok)) && (GET_CANWIN(i, j) == 0 ? false : true || XOffset > 3))
                    {
                        switch (XOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD1);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb3);

                                break;

                            case 4:
                                PUT_POINT(i, j, (int)POINT.P_GOOD4);
                                break;
                        }
                    }
                }
            }

            CheckCanWinBoard(Player, (int)DIR.DIR_TB); //위작업을 위아래로 반복

            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset, j + YOffset + 1) == AIPlayerNumber; ++YOffset) ;
                    EOLX1 = i + XOffset;

                    EOLY1 = j + YOffset + 1;
                    k = YOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset, j - YOffset - 1) == AIPlayerNumber; ++YOffset) ;
                    EOLX2 = i - XOffset;

                    EOLY2 = j - YOffset - 1;
                    YOffset += k;

                    if ((YOffset < 5 || (YOffset >= 5 && OverOmok)) && (GET_CANWIN(i, j) == 0 ? false : true || YOffset > 3))
                    {
                        switch (YOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD1);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb3);

                                break;

                            case 4:
                                PUT_POINT(i, j, (int)POINT.P_GOOD4);
                                break;
                        }
                    }
                }
            }

            CheckCanWinBoard(Player, (int)DIR.DIR_RTLB); //대각선으로 위의작업 반복
            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset - 1, j + YOffset + 1) == AIPlayerNumber; --XOffset, ++YOffset) ;
                    EOLX1 = i + XOffset - 1;

                    EOLY1 = j + YOffset + 1;
                    k = YOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset + 1, j - YOffset - 1) == AIPlayerNumber; --XOffset, ++YOffset) ;
                    EOLX2 = i - XOffset + 1;

                    EOLY2 = j - YOffset - 1;
                    YOffset += k;

                    if ((YOffset < 5 || (YOffset >= 5 && OverOmok)) && (GET_CANWIN(i, j) == 0 ? false : true || YOffset > 3))
                    {
                        switch (YOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD1);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_GOOD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_GOODb3);

                                break;

                            case 4:
                                PUT_POINT(i, j, (int)POINT.P_GOOD4);
                                break;
                        }
                    }
                }
            }
            ///////////////////////////적의 돌에대한 점수 체크

            CheckCanWinBoard(AIPlayerNumber, (int)DIR.DIR_LR); //가로체크
            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset + 1, j + YOffset) == Player; ++XOffset) ;
                    EOLX1 = i + XOffset + 1;

                    EOLY1 = j + YOffset;
                    k = XOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset - 1, j - YOffset) == Player; ++XOffset) ;
                    EOLX2 = i - XOffset - 1;

                    EOLY2 = j - YOffset;
                    XOffset += k;

                    if ((XOffset < 5 || (XOffset >= 5 && OverOmok)) && (GET_CANWIN(i, j) == 0 ? false : true || XOffset > 3))
                    {
                        switch (XOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD1);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb3);

                                break;

                            case 4:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD4);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb4);

                                break;
                        }
                    }
                }
            }


            CheckCanWinBoard(AIPlayerNumber, (int)DIR.DIR_LTRB); //대각선 체크
            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset + 1, j + YOffset + 1) == Player; ++XOffset, ++YOffset) ;
                    EOLX1 = i + XOffset + 1;

                    EOLY1 = j + YOffset + 1;
                    k = XOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset - 1, j - YOffset - 1) == Player; ++XOffset, ++YOffset) ;
                    EOLX2 = i - XOffset - 1;

                    EOLY2 = j - YOffset - 1;
                    XOffset += k;

                    if ((XOffset < 5 || (XOffset >= 5 && OverOmok)) && (GET_CANWIN(i, j) == 0 ? false : true || XOffset > 3))
                    {
                        switch (XOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD1);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb3);

                                break;

                            case 4:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD4);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb4);

                                break;
                        }
                    }
                }
            }


            CheckCanWinBoard(AIPlayerNumber, (int)DIR.DIR_TB); // 위아래방향 체크
            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset, j + YOffset + 1) == Player; ++YOffset) ;
                    EOLX1 = i + XOffset;

                    EOLY1 = j + YOffset + 1;
                    k = YOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset, j - YOffset - 1) == Player; ++YOffset) ;
                    EOLX2 = i - XOffset;

                    EOLY2 = j - YOffset - 1;
                    YOffset += k;

                    if ((YOffset < 5 || (YOffset >= 5 && OverOmok)) && (GET_CANWIN(i, j) == 0 ? false : true || YOffset > 3))
                    {
                        switch (YOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD1);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb3);

                                break;

                            case 4:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD4);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb4);

                                break;
                        }
                    }
                }
            }


            CheckCanWinBoard(AIPlayerNumber, (int)DIR.DIR_RTLB); //대각선 체크
            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i) //보드전체를 돌면서 점수 체크 시작
                {
                    if (CheckBoard(i, j) != 0) // 현재보드에 돌이 있다면 루프를 헛돈다
                        continue;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i + XOffset - 1, j + YOffset + 1) == Player; --XOffset, ++YOffset) ;
                    EOLX1 = i + XOffset - 1;

                    EOLY1 = j + YOffset + 1;
                    k = YOffset;

                    for (XOffset = 0, YOffset = 0; CheckBoard(i - XOffset + 1, j - YOffset - 1) == Player; --XOffset, ++YOffset) ;
                    EOLX2 = i - XOffset + 1;

                    EOLY2 = j - YOffset - 1;
                    YOffset += k;

                    if ((YOffset < 5 || (YOffset >= 5 && OverOmok)) && (GET_CANWIN(i, j) == 0 ? false : true || YOffset > 3))
                    {
                        switch (YOffset)
                        {
                            case 1:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD1);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb1);

                                break;

                            case 2:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD2);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb2);

                                break;

                            case 3:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD3);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb3);

                                break;

                            case 4:
                                if (CheckBoard(EOLX1, EOLY1) == 0 && CheckBoard(EOLX2, EOLY2) == 0)
                                    PUT_POINT(i, j, (int)POINT.P_BAD4);

                                else
                                    PUT_POINT(i, j, (int)POINT.P_BADb4);

                                break;
                        }
                    }
                }
            }

            for (j = 0; j < 19; ++j)
            {
                for (i = 0; i < 19; ++i)
                {
                    k += GET_POINT(i, j); //점수를 다더해도 0이면 점수가 없다는 뜻
                }
            }

            if (k == 0)
            {// 0이면 
                for (j = 0; j < 19; ++j)
                {
                    for (i = 0; i < 19; ++i)
                    {
                        if (CheckBoard(i, j) == 0) // 돌이없는곳중 아무데나
                            PUT_POINT(i, j, (int)POINT.P_GOOD0);
                    }
                }
            }

            AI_GetMaxPointRandom(ref CursorX, ref CursorY); //점수판 중 최대크기의 포인터에다 
        }
    }
}
