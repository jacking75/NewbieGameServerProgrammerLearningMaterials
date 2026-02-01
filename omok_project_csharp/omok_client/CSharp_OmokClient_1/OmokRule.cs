using System;
using System.Collections.Generic;
using System.Drawing;

namespace CSCommon
{
    public class OmokRule
    {
        enum 돌종류 { 없음, 흑돌, 백돌 };

        const int 바둑판크기 = 19;
        int[,] 바둑판 = new int[바둑판크기, 바둑판크기];
        bool 흑돌차례 = true;
        bool 게임종료 = false;
        //bool 삼삼 = false;
        
        //bool AI모드 = true;
        //돌종류 컴퓨터돌;

        //private int 전x좌표 = -1, 전y좌표 = -1;
        private int 전돌x좌표 = -1, 전돌y좌표 = -1;
        private int 현재돌x좌표 = -1, 현재돌y좌표 = -1;
        private Stack<Point> st = new Stack<Point>();

        public void StartGame()
        {
            Array.Clear(바둑판, 0, 바둑판크기 * 바둑판크기);
            //전x좌표 = 전y좌표 = -1;
            전돌x좌표 = 전돌y좌표 = -1;
            현재돌x좌표 = 현재돌y좌표 = -1;
            흑돌차례 = true;
            //삼삼 = false;
            게임종료 = false;
            
            st.Clear();            
        }

        public void 돌두기(int x, int y)
        {
            if (흑돌차례)
            {   // 검은 돌
                바둑판[x, y] = (int)돌종류.흑돌;
            }
            else
            {
                // 흰 돌
                바둑판[x, y] = (int)돌종류.백돌;
            }

            if (삼삼확인(x, y) && 흑돌차례)
            {
                //오류효과음.Play();
                //MessageBox.Show("금수자리입니다. \r다른곳에 놓아주세요.", "금수 - 쌍삼");
                바둑판[x, y] = (int)돌종류.없음;
                //삼삼 = true;
                return;
            }
            else
            {
                전돌x좌표 = 현재돌x좌표;
                전돌y좌표 = 현재돌y좌표;

                현재돌x좌표 = x;
                현재돌y좌표 = y;

                //삼삼 = false;
                흑돌차례 = !흑돌차례;                   // 차례 변경

                //바둑돌소리.Play();
            }
        }


        void 한수무르기()
        {
            st.Pop();
            바둑판[현재돌x좌표, 현재돌y좌표] = (int)돌종류.없음;

            if (st.Count != 0)
            {
                현재돌x좌표 = st.Peek().X;
                현재돌y좌표 = st.Peek().Y;
            }
            else
            {
                현재돌x좌표 = 현재돌y좌표 = -1;
            }
        }

        void 무르기(object sender, EventArgs e)
        {
            if (!게임종료 && st.Count != 0)
            {
                /*무르기요청.Play();

                if (MessageBox.Show("한 수 무르시겠습니까?", "무르기", MessageBoxButtons.YesNo) == DialogResult.Yes) // MessageBox 띄워서 무르기 여부 확인하고 예를 누르면
                {
                    if (AI모드)
                    {
                        한수무르기();
                        한수무르기();
                    }

                    else
                    {
                        한수무르기();
                        흑돌차례 = !흑돌차례;
                    }


                    panel1.Invalidate();
                }*/
            }
        }


        public void 오목확인(int x, int y)
        {
            if (가로확인(x, y) == 5)        // 같은 돌 개수가 5개면 (6목이상이면 게임 계속) 
            {
                //승리효과음.Play();
                //MessageBox.Show((돌종류)바둑판[x, y] + " 승");
                게임종료 = true;
            }

            else if (세로확인(x, y) == 5)
            {
                //승리효과음.Play();
                //MessageBox.Show((돌종류)바둑판[x, y] + " 승");
                게임종료 = true;
            }

            else if (사선확인(x, y) == 5)
            {
                //승리효과음.Play();
                //MessageBox.Show((돌종류)바둑판[x, y] + " 승");
                게임종료 = true;
            }

            else if (역사선확인(x, y) == 5)
            {
                //승리효과음.Play();
                //MessageBox.Show((돌종류)바둑판[x, y] + " 승");
                게임종료 = true;
            }
        }

        int 가로확인(int x, int y)      // ㅡ 확인
        {
            int 같은돌개수 = 1;

            for (int i = 1; i <= 5; i++)
            {
                if (x + i <= 18 && 바둑판[x + i, y] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            for (int i = 1; i <= 5; i++)
            {
                if (x - i >= 0 && 바둑판[x - i, y] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            return 같은돌개수;
        }

        int 세로확인(int x, int y)      // | 확인
        {
            int 같은돌개수 = 1;

            for (int i = 1; i <= 5; i++)
            {
                if (y + i <= 18 && 바둑판[x, y + i] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            for (int i = 1; i <= 5; i++)
            {
                if (y - i >= 0 && 바둑판[x, y - i] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            return 같은돌개수;
        }

        int 사선확인(int x, int y)      // / 확인
        {
            int 같은돌개수 = 1;

            for (int i = 1; i <= 5; i++)
            {
                if (x + i <= 18 && y - i >= 0 && 바둑판[x + i, y - i] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            for (int i = 1; i <= 5; i++)
            {
                if (x - i >= 0 && y + i <= 18 && 바둑판[x - i, y + i] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            return 같은돌개수;
        }

        int 역사선확인(int x, int y)     // ＼ 확인
        {
            int 같은돌개수 = 1;

            for (int i = 1; i <= 5; i++)
            {
                if (x + i <= 18 && y + i <= 18 && 바둑판[x + i, y + i] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            for (int i = 1; i <= 5; i++)
            {
                if (x - i >= 0 && y - i >= 0 && 바둑판[x - i, y - i] == 바둑판[x, y])
                    같은돌개수++;

                else
                    break;
            }

            return 같은돌개수;
        }

        bool 삼삼확인(int x, int y)     // 33확인
        {
            int 삼삼확인 = 0;

            삼삼확인 += 가로삼삼확인(x, y);
            삼삼확인 += 세로삼삼확인(x, y);
            삼삼확인 += 사선삼삼확인(x, y);
            삼삼확인 += 역사선삼삼확인(x, y);

            if (삼삼확인 >= 2)
                return true;

            else
                return false;
        }

        int 가로삼삼확인(int x, int y)    // 가로 (ㅡ) 확인
        {
            int 돌3개확인 = 1;
            int i, j;

            for (i = 1; i <= 3; i++) // 돌을 둔 위치로부터 → 확인
            {
                if (x + i > 18)
                    break;

                else if (바둑판[x + i, y] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x + i, y] != (int)돌종류.없음)
                    break;
            }

            for (j = 1; j <= 3; j++) // 돌을 둔 위치로부터 ← 확인
            {
                if (x - j < 0)
                    break;

                else if (바둑판[x - j, y] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x - j, y] != (int)돌종류.없음)
                    break;
            }

            if (돌3개확인 == 3 && x + i < 18 && x - j > 0)    //돌 개수가 3개면서 양쪽 벽에 붙어잇으면 안된다
            {
                if ((바둑판[x + i, y] != (int)돌종류.없음 && 바둑판[x + i - 1, y] != (int)돌종류.없음) || (바둑판[x - j, y] != (int)돌종류.없음 && 바둑판[x - j + 1, y] != (int)돌종류.없음))
                {
                    return 0;
                }

                else
                    return 1;
            }

            return 0;
        }

        private int 세로삼삼확인(int x, int y)    // 세로 (|) 확인
        {
            int 돌3개확인 = 1;
            int i, j;

            돌3개확인 = 1;

            for (i = 1; i <= 3; i++) // 돌을 둔 위치로부터 ↓ 확인
            {
                if (y + i > 18)
                    break;

                else if (바둑판[x, y + i] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x, y + i] != (int)돌종류.없음)
                    break;
            }

            for (j = 1; j <= 3; j++) // 돌을 둔 위치로부터 ↑ 확인
            {
                if (y - j < 0)
                    break;

                else if (바둑판[x, y - j] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x, y - j] != (int)돌종류.없음)
                    break;
            }

            if (돌3개확인 == 3 && y + i < 18 && y - j > 0)    //돌 개수가 3개면서 양쪽 벽에 붙어잇으면 안된다
            {
                if ((바둑판[x, y + i] != (int)돌종류.없음 && 바둑판[x, y + i - 1] != (int)돌종류.없음) || (바둑판[x, y - j] != (int)돌종류.없음 && 바둑판[x, y - j + 1] != (int)돌종류.없음))
                {
                    return 0;
                }

                else
                    return 1;
            }

            return 0;
        }

        int 사선삼삼확인(int x, int y)    // 사선 (/) 확인
        {
            int 돌3개확인 = 1;
            int i, j;

            돌3개확인 = 1;

            for (i = 1; i <= 3; i++) // 돌을 둔 위치로부터 ↗ 확인
            {
                if (x + i > 18 || y - i < 0)
                    break;

                else if (바둑판[x + i, y - i] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x + i, y - i] != (int)돌종류.없음)
                    break;
            }

            for (j = 1; j <= 3; j++) // 돌을 둔 위치로부터 ↙ 확인
            {
                if (x - j < 0 || y + j > 18)
                    break;

                else if (바둑판[x - j, y + j] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x - j, y + j] != (int)돌종류.없음)
                    break;
            }

            if (돌3개확인 == 3 && x + i < 18 && y - i > 0 && x - j > 0 && y + j < 18)    //돌 개수가 3개면서 양쪽 벽에 붙어잇으면 안된다
            {
                if ((바둑판[x + i, y - i] != (int)돌종류.없음 && 바둑판[x + i - 1, y - i + 1] != (int)돌종류.없음) || (바둑판[x - j, y + j] != (int)돌종류.없음 && 바둑판[x - j + 1, y + j - 1] != (int)돌종류.없음))
                {
                    return 0;
                }

                else
                    return 1;
            }

            return 0;
        }

        int 역사선삼삼확인(int x, int y)    // 역사선 (＼) 확인
        {
            int 돌3개확인 = 1;
            int i, j;

            돌3개확인 = 1;

            for (i = 1; i <= 3; i++) // 돌을 둔 위치로부터 ↘ 확인
            {
                if (x + i > 18 || y + i > 18)
                    break;

                else if (바둑판[x + i, y + i] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x + i, y + i] != (int)돌종류.없음)
                    break;
            }

            for (j = 1; j <= 3; j++) // 돌을 둔 위치로부터 ↖ 확인
            {
                if (x - j < 0 || y - j < 0)
                    break;

                else if (바둑판[x - j, y - j] == 바둑판[x, y])
                    돌3개확인++;

                else if (바둑판[x - j, y - j] != (int)돌종류.없음)
                    break;
            }

            if (돌3개확인 == 3 && x + i < 18 && y + i < 18 && x - j > 0 && y - j > 0)    //돌 개수가 3개면서 양쪽 벽에 붙어잇으면 안된다
            {
                if ((바둑판[x + i, y + i] != (int)돌종류.없음 && 바둑판[x + i - 1, y + i - 1] != (int)돌종류.없음) || (바둑판[x - j, y - j] != (int)돌종류.없음 && 바둑판[x - j + 1, y - j + 1] != (int)돌종류.없음))
                {
                    return 0;
                }

                else
                    return 1;
            }

            return 0;
        }
    }
}
