
#include "NETCore.h"

namespace Atomic
{

SharedPtr<Context> NETCore::csContext_;

NETCore::NETCore(Context* context) :
    Object(context)
{
    assert (!csContext_);
    csContext_ = context;
}

NETCore::~NETCore()
{
}

NETCore* NETCore::Initialize()
{
    Context* context = new Context();
    NETCore* netCore = new NETCore(context);
    context->RegisterSubsystem(netCore);
    return netCore;
}

void NETCore::Shutdown()
{
    csContext_ = nullptr;
}

extern "C"
{
#ifdef ATOMIC_PLATFORM_WINDOWS
#define ATOMIC_EXPORT_API __declspec(dllexport)
#else
#define ATOMIC_EXPORT_API
#endif

ATOMIC_EXPORT_API ClassID csb_Atomic_RefCounted_GetClassID(RefCounted* refCounted)
{
    if (!refCounted)
        return 0;

    return refCounted->GetClassID();
}

}

}

