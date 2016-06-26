using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace AtomicEngine
{
    public class NativeType 
    {

		public Type Type => type;

        public NativeType(IntPtr nativeClassID, Type type, Func<IntPtr, RefCounted> managedConstructor)
        {
            this.nativeClassID = nativeClassID;
            this.type = type;
            this.managedConstructor = managedConstructor;

            NativeCore.RegisterNativeType(this);
        }

        internal Type type;
        internal IntPtr nativeClassID;
        internal Func<IntPtr, RefCounted> managedConstructor;

    }

    static class NativeCore
    {
        
        // register a newly created native
        public static IntPtr RegisterNative (IntPtr native, RefCounted r)
        {
            return native;
        }

        public static void RegisterNativeType(NativeType nativeType)
        {
            if (nativeClassIDToNativeType.ContainsKey(nativeType.nativeClassID) || typeToNativeType.ContainsKey(nativeType.type))
            {
                throw new System.InvalidOperationException("NativeCore.RegisterNativeType - Duplicate NativeType Registered");
            }

            nativeClassIDToNativeType[nativeType.nativeClassID] = nativeType;
            typeToNativeType[nativeType.type] = nativeType;

        } 

        // wraps an existing native instance, with downcast support
        public static T WrapNative<T> (IntPtr native) where T:RefCounted
        {
            return null;
        }

        // Native ClassID to NativeType lookup
        internal static Dictionary<IntPtr,NativeType> nativeClassIDToNativeType = new Dictionary<IntPtr, NativeType>();

        // Managed Type to NativeType lookup
        internal static Dictionary<Type,NativeType> typeToNativeType = new Dictionary<Type, NativeType>();
    }

    public partial class RefCounted
    {

        public RefCounted()
        {
        }

        protected RefCounted(IntPtr native)
        {
            nativeInstance = native;
        }

        public IntPtr nativeInstance = IntPtr.Zero;

        [DllImport(Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr csb_Atomic_RefCounted_GetClassID(IntPtr self);

    }

    public static partial class Constants
    {
        public const string LIBNAME = "/Users/josh/Dev/atomic/build-AtomicGameEngine-Desktop-Debug/Source/AtomicNET/AtomicNETNative/libAtomicNETNative.dylib";
    }
    public partial class NETApplication
    {

        public static int RunMain()
        {
            return csb_Atomic_NETApplication_Main();
        }

        [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
         private static extern int csb_Atomic_NETApplication_Main();
    }


}
