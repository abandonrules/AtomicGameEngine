
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AtomicEngine;

namespace AtomicEngine

{

[StructLayout(LayoutKind.Sequential)]
public struct CoreDelegates
{
    [MarshalAs(UnmanagedType.FunctionPtr)]
    public EventDispatchDelegate eventDispatch;
}

public partial class NETCore : AObject
{
    public static NETCore Initialize()
    {

        AtomicNET.Initialize();

        // AtomicNET Modules
        AtomicNETModule.Initialize();

        NativeCore.RegisterNETEventType += RegisterNETEventType;    

        CoreDelegates delegates = new CoreDelegates();
        delegates.eventDispatch = NativeCore.EventDispatch;

        IntPtr coreptr = csb_Atomic_NETCore_Initialize(ref delegates);

        NETCore core = (coreptr == IntPtr.Zero ? null :  NativeCore.WrapNative<NETCore> (coreptr));

        if (core != null)
            AtomicNET.RegisterSubsystem("NETCore", core);

    
        AtomicNET.SetContext (core.Context);    

        return core; 
    }

    [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    private static extern IntPtr csb_Atomic_NETCore_Initialize(ref CoreDelegates delegates);

}


}