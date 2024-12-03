namespace AutoTestClient.Dummy
{
    public class DummyRoom : DummyNetwork
    {
        public Int32 EnteredRoomNumber { get; protected set; } = -1;

        public List<string> OtherUserIDList { get; protected set; }

        public string SendedChatMessage { get; protected set; } = string.Empty;

        public ErrorCode AddOtherUserID(string otherUserID)
        {
            if (OtherUserIDList.Contains(otherUserID) == true)
            {
                return ErrorCode.AlreadyRoomUserID;
            }

            if (OtherUserIDList.Count == OtherUserIDList.Capacity)
            {
                return ErrorCode.FailedRoomEnterIsFullRoom;
            }

            OtherUserIDList.Add(otherUserID);

            return ErrorCode.None;
        }

        public bool RemoveOtherUserID(string otherUserID)
        {
            return OtherUserIDList.Remove(otherUserID);
        }

        protected void ReadyRoomChatAction()
        {
            SendedChatMessage = $"TEST_CHAT_MESSAGE_{Number}";
        }
    }
}
