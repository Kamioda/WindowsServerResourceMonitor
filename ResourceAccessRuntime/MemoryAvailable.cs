namespace ResourceAccessRuntime
{
    class MemoryAvailable : PerfCounterBase
    {
        public MemoryAvailable() : base("Memory", "Available MBytes") { }
        public override string Get()
        {
            return base.Get();
        }
    }
}
