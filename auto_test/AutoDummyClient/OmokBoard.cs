namespace AutoTestClient
{
    public class OmokBoard
    {
        public int Height { get; private set; }

        public int Width { get; private set; }

        private StoneType[,] _board;

        public int Size() => _board.Length;

        public void Init(int width, int height)
        {
            Width = width;
            Height = height;

            _board = new StoneType[Height, Width];
        }

        public void Prepare()
        {
            Clear();
        }

        public void Clear()
        {
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    _board[y, x] = StoneType.Empty;
                }
            }
        }

        public List<(int Y, int X)> GetEmptyPosition()
        {
            var results = new List<(int, int)>();

            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    if (_board[y, x] == StoneType.Empty)
                    {
                        results.Add((y, x));
                    }
                }
            }

            return results;
        }

        public void 돌놓기(StoneType stoneType, int x, int y)
        {
            _board[y, x] = stoneType;
        }
    }

    public enum StoneType : byte
    {
        Empty = 0,
        White = 1,
        Black = 2,
    }

    public enum 돌놓기결과 : short
    {
        None = 0,
        NotEmpty,
        GameDone,
    }
}
