using System.Diagnostics;

namespace ResourceAccessRuntime
{
    class PerfCounterBase
    {
        public string CategoryName;
        public string CounterName;
        public string instanceName;
        private PerformanceCounter counter;
        public PerfCounterBase(string CategoryName, string CounterName, string instanceName)
        {
            this.CategoryName = CategoryName;
            this.CounterName = CounterName;
            this.instanceName = instanceName;
            counter = new PerformanceCounter(CategoryName, CounterName, instanceName);
        }
        public PerfCounterBase(string CategoryName, string CounterName)
        {
            this.CategoryName = CategoryName;
            this.CounterName = CounterName;
            instanceName = "";
            counter = new PerformanceCounter(CategoryName, CounterName);
        }
        protected float GetValue() { return counter.NextValue(); }
        protected long GetRaw() { return counter.RawValue; }
        public virtual string Get() 
        {
            int v = (int)(GetValue() * 100 + 0.5);
            return (v / 100f).ToString();
        }
    }

}
