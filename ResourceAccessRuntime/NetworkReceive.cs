namespace ResourceAccessRuntime
{
    class NetworkReceive : PerfCounterBase
    {
        public NetworkReceive(string NetworkDeviceName) : base("Network Adapter", "Bytes Received/sec", NetworkDeviceName) { }
        public override string Get()
        {
            return base.Get();
        }
    }
}
