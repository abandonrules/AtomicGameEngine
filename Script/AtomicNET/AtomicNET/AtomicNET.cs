
using System;
using System.Collections.Generic;
using AtomicEngine;

public static class AtomicNET
{

    public static Context Context => context;

    public static T GetSubsystem<T>() where T :AObject
    {
        return (T) subSystems [typeof(T)];
    }

    public static void RegisterSubsystem (String name)
    {
        var subsystem = AtomicNET.Context.GetSubsystem(name);
        
        if (subsystem == null)
        {
            throw new System.InvalidOperationException("AtomicNET.RegisterSubsystem - Attempting to register null subsystem");
        }  

        subSystems[subsystem.GetType()] = subsystem;
    }
    
    public static void Initialize() 
    {
        // Atomic Modules
        CoreModule.Initialize ();
        MathModule.Initialize ();
        EngineModule.Initialize ();
        InputModule.Initialize ();
        IOModule.Initialize ();
        ResourceModule.Initialize ();
        AudioModule.Initialize ();
        GraphicsModule.Initialize ();
        SceneModule.Initialize ();
        Atomic2DModule.Initialize ();
        Atomic3DModule.Initialize ();
        NavigationModule.Initialize ();
        NetworkModule.Initialize ();
        PhysicsModule.Initialize ();
        EnvironmentModule.Initialize ();
        UIModule.Initialize ();

        // AtomicNET Modules
        AtomicNETModule.Initialize();    

        // Initialize Atomic NET Core, creates Context, etc
        netCore = NETCore.Initialize();
        context = netCore.Context;             	
        
    }

    private static NETCore netCore;
    private static Context context;
    private static Dictionary<Type, AObject> subSystems = new Dictionary<Type, AObject>();
    
}
