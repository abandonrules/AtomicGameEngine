using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace AtomicEngine
{

static class NativeCore
{
    // wraps an existing native instance, with downcast support
    public static T WrapNative<T> (IntPtr native) where T:RefCounted
    {
        return null;
    }

    public static void RegisterNativeType (Type type)
    {
      
    }

    public static bool GetNativeType (Type type)
    {
      return true;
    }


    public static IntPtr RegisterNative (IntPtr native, RefCounted r)
    {
        return IntPtr.Zero;
    }

    public static Dictionary<IntPtr, Func<IntPtr, RefCounted>> nativeClassIDToManagedConstructor = new Dictionary<IntPtr, Func<IntPtr, RefCounted>>();
}

public partial class RefCounted
{

    public RefCounted ()
    {
    }

    protected RefCounted (IntPtr native)
    {
        nativeInstance = native;
    }

    public IntPtr nativeInstance = IntPtr.Zero;

    [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static extern IntPtr csb_Atomic_RefCounted_GetClassID (IntPtr self);

}

public static partial class Constants
{
    public const string LIBNAME = "/Users/josh/Dev/atomic/build-AtomicGameEngine-Desktop-Debug/Source/AtomicNET/AtomicNETNative/libAtomicNETNative.dylib";
}


}
