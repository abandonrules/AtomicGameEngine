using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace AtomicEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct CoreDelegates
    {
        [MarshalAs(UnmanagedType.FunctionPtr)]
        public EventDispatchDelegate eventDispatch;
    }

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

        static internal void SubscribeToEvent(AObject receiver, uint eventType)
        {
            List<WeakReference> eventReceivers;

            if (!eventReceiverLookup.TryGetValue (eventType, out eventReceivers)) {
                eventReceivers = eventReceiverLookup[eventType] = new List<WeakReference>(); 
            }

            foreach(WeakReference wr in eventReceivers)
            {
                // GC'd?
                if (!wr.IsAlive)
                    continue;
                
                // already on list?
                if (((AObject) wr.Target) == receiver)
                    return;
            }

            WeakReference w = null;

            if (!nativeLookup.TryGetValue(receiver.nativeInstance, out w))
            {
                throw new System.InvalidOperationException("NativeCore.SubscribeToEvent - unable to find native instance");
            }

            if (!w.IsAlive)
            {
                throw new System.InvalidOperationException("NativeCore.SubscribeToEvent - attempting to subscribe a GC'd AObject");
            }

            eventReceivers.Add(w);        
        }

        static ScriptVariantMap[] svm;
        static int svmDepth = 0;
        const int svmMax = 256;

        internal static void Initialize()
        {
            // preallocate script variant maps
            svm = new ScriptVariantMap[svmMax];
            for (int i = 0; i < svmMax; i++)
                svm[i] = new ScriptVariantMap();
        }

        public static void EventDispatch(uint eventType, IntPtr eventData)
        {
            List<WeakReference> eventReceivers;

            if (!eventReceiverLookup.TryGetValue (eventType, out eventReceivers)) {

                // This should not happen, as event NET objects are subscribed to are filtered 
                throw new System.InvalidOperationException("NativeCore.EventDispatch - received unregistered event type");
            
            }

            ScriptVariantMap scriptMap = null;

            foreach(var w in eventReceivers)
            {
                // GC'd?
                if (!w.IsAlive)
                    continue;

                if (scriptMap == null)
                {
                    if (svmDepth == svmMax)
                    {
                        throw new System.InvalidOperationException("NativeCore.EventDispatch - exceeded max svm");
                    }

                    scriptMap = svm[svmDepth++];
                    scriptMap.CopyVariantMap(eventData);
                }

                ((AObject)w.Target).HandleEvent(eventType, scriptMap);                    
            }

            if (scriptMap != null)
                svmDepth--;
            
        }

        // register a newly created native
        public static IntPtr RegisterNative (IntPtr native, RefCounted r)
        {
            if (nativeLookup.ContainsKey(native))
            {
                throw new System.InvalidOperationException("NativeCore.RegisterNative - Duplicate IntPtr key");
            }

            r.nativeInstance = native;

            var w = new WeakReference (r);
            nativeLookup [native] = w;

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
        internal static Dictionary<IntPtr, WeakReference> nativeLookup = new Dictionary<IntPtr, WeakReference> ();

        // Native ClassID to NativeType lookup
        internal static Dictionary<IntPtr, NativeType> nativeClassIDToNativeType = new Dictionary<IntPtr, NativeType>();

        // weak references here, hold a ref native side
        internal static Dictionary<uint, List<WeakReference>> eventReceiverLookup = new Dictionary<uint, List<WeakReference>>();

        // Managed Type to NativeType lookup
        internal static Dictionary<Type, NativeType> typeToNativeType = new Dictionary<Type, NativeType>();

        [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void csb_AtomicEngine_ReleaseRef (IntPtr refCounted);

    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EventDispatchDelegate(uint eventType, IntPtr eventData);

    public static class AtomicNET
    {

        public static Context Context => context;

        public static T GetSubsystem<T>() where T :AObject
        {
            AObject subSystem = null;
            subSystems.TryGetValue(typeof(T), out subSystem);
            return (T) subSystem;
        }

        public static void RegisterSubsystem (String name, AObject instance = null)
        {
            if (instance != null)
            {
                subSystems[instance.GetType()] = instance;
                return;
            }

            var subsystem = AtomicNET.Context.GetSubsystem(name);

            if (subsystem == null)
            {
                throw new System.InvalidOperationException("AtomicNET.RegisterSubsystem - Attempting to register null subsystem");
            }  

            subSystems[subsystem.GetType()] = subsystem;
        }

        public static uint StringToStringHash(string value)
        {
            return csb_Atomic_AtomicNET_StringToStringHash (value);
        }

        [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern uint csb_Atomic_AtomicNET_StringToStringHash(string name);

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

            AtomicNETModule.Initialize();

            CoreDelegates delegates = new CoreDelegates();
            delegates.eventDispatch = NativeCore.EventDispatch;

            IntPtr coreptr = csb_Atomic_NETCore_Initialize(ref delegates);

            NETCore core = (coreptr == IntPtr.Zero ? null : NativeCore.WrapNative<NETCore>(coreptr));

            if (core != null)
                AtomicNET.RegisterSubsystem("NETCore", core);

            context = core.Context;

            NativeCore.Initialize();

        }

        [DllImport(Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr csb_Atomic_NETCore_Initialize(ref CoreDelegates delegates);

        private static Context context;
        private static Dictionary<Type, AObject> subSystems = new Dictionary<Type, AObject>();

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

    public delegate void EventDelegate (uint eventType, ScriptVariantMap eventData);

    public partial class AObject
    {
        internal Dictionary<uint, EventDelegate> eventHandlers = new Dictionary<uint, EventDelegate>();

        internal void HandleEvent(uint eventType, ScriptVariantMap eventData)
        {
            eventHandlers[eventType](eventType, eventData);
        }

        public void SubscribeToEvent(uint eventType, EventDelegate eventDelegate)
        {
            NETCore.RegisterNETEventType(eventType);
            eventHandlers[eventType] = eventDelegate;
            NativeCore.SubscribeToEvent(this, eventType);
        }

        public void SubscribeToEvent(string eventType, EventDelegate eventDelegate)
        {
            SubscribeToEvent(AtomicNET.StringToStringHash(eventType), eventDelegate);           
        }

    }

    public partial class ScriptVariantMap
    {

        public void CopyVariantMap(IntPtr vm)
        {
            csb_Atomic_AtomicNET_ScriptVariantMapCopyVariantMap(nativeInstance, vm);
        }

        [DllImport (Constants.LIBNAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern IntPtr csb_Atomic_AtomicNET_ScriptVariantMapCopyVariantMap(IntPtr svm, IntPtr vm);
        
    }

    public static partial class Constants
    {
        public const string LIBNAME = "AtomicNETNative.dll";
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



}
