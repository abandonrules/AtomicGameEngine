using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace AtomicEngine
{

    public partial class NETApplication : Application
    {

        public static NETApplication Create()
        {
            // Initialize AtomicNET
            AtomicNET.Initialize();

            var app = CreateInternal();

            app.Initialize();

            AtomicNET.RegisterSubsystem ("Graphics");

            return app;
        }
                
    }

}