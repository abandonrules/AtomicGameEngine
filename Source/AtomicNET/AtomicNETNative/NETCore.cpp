
#include <Atomic/Math/MathDefs.h>
#include <Atomic/Core/ProcessUtils.h>

#include "NETCore.h"
#include "NETEventHelper.h"

namespace Atomic
{


SharedPtr<Context> NETCore::csContext_;
NETCoreEventDispatchFunction NETCore::eventDispatch_ = nullptr;

NETCore::NETCore(Context* context, NETCoreDelegates* delegates) :
    Object(context)
{
    assert (!csContext_);
    csContext_ = context;

    eventDispatch_ = delegates->eventDispatch;

    NETEventDispatcher* dispatcher = new NETEventDispatcher(context_);
    context_->RegisterSubsystem(dispatcher);
    context_->AddGlobalEventListener(dispatcher);
}

NETCore::~NETCore()
{
    assert (!csContext_);
}

void NETCore::RegisterNETEventType(unsigned eventType)
{
    NETEventDispatcher* dispatcher = csContext_->GetSubsystem<NETEventDispatcher>();
    dispatcher->RegisterNETEvent(StringHash(eventType));
}

void NETCore::Shutdown()
{
    assert (csContext_);

    csContext_->RemoveGlobalEventListener(csContext_->GetSubsystem<NETEventDispatcher>());
    csContext_->RemoveSubsystem(NETEventDispatcher::GetTypeStatic());

    eventDispatch_ = nullptr;
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

ATOMIC_EXPORT_API ClassID csb_Atomic_NETCore_Initialize(NETCoreDelegates* delegates)
{
    Context* context = new Context();
    NETCore* netCore = new NETCore(context, delegates);
    context->RegisterSubsystem(netCore);
    return netCore;
}

unsigned csb_Atomic_AtomicNET_StringToStringHash(const char* str)
{
    unsigned hash = 0;

    if (!str)
        return hash;

    while (*str)
    {
        // Perform the actual hashing as case-insensitive
        char c = *str;
        hash = SDBMHash(hash, (unsigned char)tolower(c));
        ++str;
    }

    return hash;

}

}

}

