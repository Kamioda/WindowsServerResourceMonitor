namespace ResourceAccessRuntime
{
    class MemoryUsage : PerfCounterBase
    {
        public MemoryUsage() : base("Memory", "% Committed Bytes In Use") { }
        public override string Get()
        {
            int v = 10000 - (int)(GetValue() * 100 + 0.5);
            return (v / 100f).ToString();
        }
    }
}
