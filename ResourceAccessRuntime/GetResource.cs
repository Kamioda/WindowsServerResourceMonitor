using System;
using System.Management;
using Newtonsoft.Json;

namespace ResourceAccessRuntime
{
    public class GetResource
    {
        private class MemoryInformation
        {
            [JsonProperty("max")]
            public string MaxMemorySize;
            [JsonProperty("available")]
            public string AvailableMemorySize;
            [JsonProperty("usage")]
            public string MemoryUsage;
        }
        private class DiskInformation
        {
            [JsonProperty("write")]
            public string Write;
            [JsonProperty("read")]
            public string Read;
            [JsonProperty("free")]
            public string Usage;
        }
        private class NetworkInformation
        {
            [JsonProperty("receive")]
            public string Receive;
            [JsonProperty("send")]
            public string Send;
        }
        private class ResourceInformation
        {
            [JsonProperty("processor")]
            public string ProcessorUsage;
            [JsonProperty("memory")]
            public MemoryInformation MemoryInfo;
            [JsonProperty("disk")]
            public DiskInformation DiskInfo;
            [JsonProperty("network")]
            public NetworkInformation NetworkInfo;
        }
        private static string memSize;
        private static Processor processor;
        private static DiskFreeSpace diskFree;
        private static DiskRead diskRead;
        private static DiskWrite diskWrite;
        private static MemoryAvailable memAvailable;
        private static MemoryUsage memUsage;
        private static NetworkReceive netReceive;
        private static NetworkSend netSend;
        private static ResourceInformation resource;
        public static string Init(string MonitorDrive, string NetworkDevice)
        {
            try
            {
                resource = new ResourceInformation();
                ManagementClass mc = new ManagementClass("Win32_OperatingSystem");
                ManagementObjectCollection moc = mc.GetInstances();
                foreach (ManagementObject mo in moc) { resource.MemoryInfo.MaxMemorySize = (int.Parse(mo["TotalVisibleMemorySize"].ToString()) / 1024).ToString(); }
                diskFree = new DiskFreeSpace(MonitorDrive);
                diskRead = new DiskRead(MonitorDrive);
                diskWrite = new DiskWrite(MonitorDrive);
                memAvailable = new MemoryAvailable();
                memUsage = new MemoryUsage();
                netReceive = new NetworkReceive(NetworkDevice);
                netSend = new NetworkSend(NetworkDevice);
                return null;
            }
            catch(Exception e)
            {
                return e.Message;
            }
        }
        public static string GetAllResource()
        {
            resource.DiskInfo.Read = diskRead.Get();
            resource.DiskInfo.Write = diskWrite.Get();
            resource.DiskInfo.Usage = diskFree.Get();
            resource.MemoryInfo.AvailableMemorySize = memAvailable.Get();
            resource.MemoryInfo.MemoryUsage = memUsage.Get();
            resource.NetworkInfo.Receive = netReceive.Get();
            resource.NetworkInfo.Send = netSend.Get();
            resource.ProcessorUsage = processor.Get();
            return JsonConvert.SerializeObject(resource);
        }
    }
}
