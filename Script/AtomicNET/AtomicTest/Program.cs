using System;
using AtomicEngine;

public class Program
{
    public static void Main(string[] args)
    {               

        // Create the Application
        var app = NETApplication.Create();

        // Example of getting a subsystem
        var graphics = AtomicNET.GetSubsystem<Graphics>();

        // Managed code in charge of main loop
        while (app.RunFrame())
        {

        }

        // Shut 'er down
        app.Shutdown();
    }
}


