//
// Copyright (c) 2014-2016 THUNDERBEAST GAMES LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Atomic/IO/Log.h>

#include <Atomic/IPC/IPC.h>
#include <Atomic/IPC/IPCEvents.h>
#include <Atomic/IPC/IPCBroker.h>

#include <Atomic/Core/CoreEvents.h>

#include "AtomicNETService.h"

namespace ToolCore
{

AtomicNETService::AtomicNETService(Context* context) :
    Object(context),
    brokerEnabled_(false)
{
    SubscribeToEvent(E_IPCWORKERSTART, HANDLER(AtomicNETService, HandleIPCWorkerStarted));
}

AtomicNETService::~AtomicNETService()
{

}

void AtomicNETService::HandleIPCWorkerStarted(StringHash eventType, VariantMap& eventData)
{
    VariantMap startupData;
    serviceBroker_->PostMessage(E_IPCINITIALIZE, startupData);
    brokerEnabled_ = true;
}

void AtomicNETService::HandleIPCWorkerExit(StringHash eventType, VariantMap& eventData)
{
    if (eventData[IPCWorkerExit::P_BROKER] == serviceBroker_)
    {
        serviceBroker_ = 0;
        brokerEnabled_ = false;
    }
}

void AtomicNETService::HandleIPCWorkerLog(StringHash eventType, VariantMap& eventData)
{
    using namespace IPCWorkerLog;

    // convert to a service log

    VariantMap serviceLogData;

    serviceLogData["message"]  = eventData[P_MESSAGE].GetString();
    serviceLogData["level"]  = eventData[P_LEVEL].GetInt();

    SendEvent("NETServiceLog", serviceLogData);

}

bool AtomicNETService::Start()
{
    String serviceBinary = "/usr/local/share/dotnet/dotnet";

    Vector<String> vargs;

    vargs.Push("exec");
    vargs.Push("--additionalprobingpath");
    vargs.Push("/Users/josh/.nuget/packages");
    vargs.Push("/Users/josh/Dev/atomic/AtomicGameEngine/Script/AtomicNET/AtomicNETService/bin/Debug/netcoreapp1.0/AtomicNETService.dll");

    String dump;
    dump.Join(vargs, " ");

    LOGINFOF("Launching Broker %s %s", serviceBinary.CString(), dump.CString());

    IPC* ipc = GetSubsystem<IPC>();
    serviceBroker_ = ipc->SpawnWorker(serviceBinary, vargs);

    if (serviceBroker_)
    {
        SubscribeToEvent(serviceBroker_, E_IPCWORKEREXIT, HANDLER(AtomicNETService, HandleIPCWorkerExit));
        SubscribeToEvent(serviceBroker_, E_IPCWORKERLOG, HANDLER(AtomicNETService, HandleIPCWorkerLog));
    }

    return serviceBroker_.NotNull();

}

bool AtomicNETService::GetBrokerEnabled() const
{
    return brokerEnabled_;
}

}
