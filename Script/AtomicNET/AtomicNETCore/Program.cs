using System;
using System.Threading;

namespace ConsoleApplication
{
    public class Program
    {
        public static void Main(string[] args)
        {            
            // Thread.Sleep(30 * 1000);
            var debugBuild = AtomicEngine.Engine.GetDebugBuild();
            Console.WriteLine("Debug Build: {0}", debugBuild);
        }
    }
}
