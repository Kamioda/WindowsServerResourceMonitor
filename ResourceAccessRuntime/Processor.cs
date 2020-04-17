namespace ResourceAccessRuntime
{
    class Processor : PerfCounterBase
    {
        Processor() : base("Processor", "% Processor Time", "_Total") { }
        public override string Get()
        {
            return base.Get();
        }
    }
}
