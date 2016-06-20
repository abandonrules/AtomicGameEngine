
#include "NETCore.h"

namespace Atomic
{

WeakPtr<Context> NETCore::csContext_;
WeakPtr<NETCore> NETCore::instance_;

NETCore::NETCore(Context* context) :
    Object(context)
{
    assert(!instance_);
    instance_ = this;
    csContext_ = context;
}

NETCore::~NETCore()
{
    instance_ = nullptr;
    csContext_ = nullptr;
}

}

