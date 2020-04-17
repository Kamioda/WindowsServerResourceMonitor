namespace ResourceAccessRuntime
{
    class NetworkSend : PerfCounterBase
    {
        public NetworkSend(string NetworkDeviceName) : base("Network Adapter", "Bytes Sent/sec", NetworkDeviceName) { }
        public override string Get()
        {
            return base.Get();
        }
    }
}
