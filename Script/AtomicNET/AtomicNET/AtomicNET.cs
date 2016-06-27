
using AtomicEngine;

public static class AtomicNET
{
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
        NETCore.Initialize();     
        
    }
}
