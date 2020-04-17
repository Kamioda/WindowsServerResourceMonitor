namespace ResourceAccessRuntime
{
    class DiskWrite : PerfCounterBase
    {
        public DiskWrite(string TargetDrive = "C:") : base("LogicalDisk", "Disk Writes/sec", TargetDrive) { }
        public override string Get()
        {
            return base.Get();
        }
    }
}
