using AtomicEngine;

namespace ConsoleApplication
{
    public class Program
    {
        public static void Main(string[] args)
        {               
            // Initialize AtomicNET
            AtomicNET.Initialize();

            // Create the Application
            var app = new NETApplication();

            // Run!
            app.Run();
        }
    }
}

