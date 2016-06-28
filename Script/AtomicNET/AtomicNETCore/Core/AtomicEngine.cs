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
    
    public static class NativeCore
    {
        
        // register a newly created native
        public static IntPtr RegisterNative (IntPtr native, RefCounted r)
        {
            if (nativeLookup.ContainsKey(native))
            {
                throw new System.InvalidOperationException("NativeCore.RegisterNative - Duplicate IntPtr key");
            }
            
            r.nativeInstance = native;
            
            var w = new WeakReference (r);
            NativeCore.nativeLookup [native] = w;
            
            // keep native side alive
            r.AddRef();
            
            return native;
        }
        
        
        // wraps an existing native instance, with downcast support
        public static T WrapNative<T> (IntPtr native) where T:RefCounted
        {
            if (native == IntPtr.Zero)
                return null;
            
            WeakReference w;
            
            // first see if we're already available
            if (nativeLookup.TryGetValue (native, out w)) {
                
                if (w.IsAlive) {
                    
                    // we're alive!
                    return (T)w.Target;
                    
                } else {
                    
                    // we were seen before, but have since been GC'd, remove!
                    nativeLookup.Remove (native);
                    csb_AtomicEngine_ReleaseRef(native);
                }
            }
            
            IntPtr classID = RefCounted.csb_Atomic_RefCounted_GetClassID (native);
            
            // and store, with downcast support for instance Component -> StaticModel
            // we never want to hit this path for script inherited natives

            NativeType nativeType;

            if (!nativeClassIDToNativeType.TryGetValue (classID, out nativeType)) 
            {
                throw new System.InvalidOperationException("NativeCore.WrapNative - Attempting to wrap unknown native class id");        
            }
            
            RefCounted r = nativeType.managedConstructor(native);
            w = new WeakReference (r);
            NativeCore.nativeLookup [native] = w;
            
            // store a ref, so native side will not be released while we still have a reference in managed code
            r.AddRef();
            
            return (T) r;
        }
        
        public static void RegisterNativeType(NativeType nativeType)
        {
            if (nativeClassIDToNativeType.ContainsKey(nativeType.nativeClassID))
            {
                throw new System.InvalidOperationException("NativeCore.RegisterNativeType - Duplicate NativeType class id registered");
            }
            if (typeToNativeType.ContainsKey(nativeType.type))
            {
                throw new System.InvalidOperationException("NativeCore.RegisterNativeType - Duplicate NativeType type registered");
            }
                        
            nativeClassIDToNativeType[nativeType.nativeClassID] = nativeType;
            typeToNativeType[nativeType.type] = nativeType;
            
        }
        
        // weak references here, hold a ref native side
        private static Dictionary<IntPtr, WeakReference> nativeLookup = new Dictionary<IntPtr, WeakReference> ();
        
        // Native ClassID to NativeType lookup
        internal static Dictionary<IntPtr,NativeType> nativeClassIDToNativeType = new Dictionary<IntPtr, NativeType>();
        
        // Managed Type to NativeType lookup
        internal static Dictionary<Type,NativeType> typeToNativeType = new Dictionary<Type, NativeType>();

        [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void csb_AtomicEngine_ReleaseRef (IntPtr refCounted);

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
        public const string LIBNAME = "/Users/josh/Dev/atomic/build-AtomicGameEngine-Desktop-Debug/Source/AtomicNET/libAtomicNETNative.dylib";
    }
    

    public class CSComponent : AObject
    {

    }

	public class InspectorAttribute : Attribute
	{
		public InspectorAttribute(string DefaultValue = "")
		{
		}

		public string DefaultValue;
	}    

/*
    public partial class NETApplication
    {
        
        public static int RunMain()
        {
            return csb_Atomic_NETApplication_Main();
        }
        
        [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int csb_Atomic_NETApplication_Main();
    }
*/    
    
}
