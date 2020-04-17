namespace ResourceAccessRuntime
{
    class DiskFreeSpace : PerfCounterBase 
    {
        public DiskFreeSpace(string TargetDrive = "C:") : base("LogicalDisk", "% Free Space", TargetDrive) { }
        public override string Get()
        {
            return base.Get();
        }
    }
}
