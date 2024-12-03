using AutoTestClient.Dummy;

namespace AutoTestClient.Network.S2CPacketHandler
{
    public abstract class BaseHandler
    {
        public static Func<Int32, DummyObject> GetDummyByIndexFunc;

        public static Func<string, DummyObject> GetDummyByIDFunc;

        public abstract void Handle(DummyObject dummy, byte[] packet);
    }
}