using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace AtomicEngine
{

    public partial class NETApplication : Application
    {

        public static NETApplication Create(bool headless = false)
        {
            // Initialize AtomicNET
            AtomicNET.Initialize();

            var app = CreateInternal(headless);

            app.Initialize();

            if (!headless)
                AtomicNET.RegisterSubsystem ("Graphics");

            return app;
        }
                
    }

}