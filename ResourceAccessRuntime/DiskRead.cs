namespace ResourceAccessRuntime
{
    class DiskRead : PerfCounterBase
    {
        public DiskRead(string TargetDrive = "C:") : base("LogicalDisk", "Disk Reads/sec", TargetDrive) { }
        public override string Get()
        {
            return base.Get();
        }
    }
}
