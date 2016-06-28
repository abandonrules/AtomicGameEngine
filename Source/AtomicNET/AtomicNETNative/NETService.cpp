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

#include <Atomic/IO/IOEvents.h>
#include <Atomic/IO/Log.h>
#include <Atomic/Core/ProcessUtils.h>
#include <Atomic/Core/CoreEvents.h>
#include <Atomic/IPC/IPCEvents.h>
#include <Atomic/IPC/IPCWorker.h>

#include "NETService.h"

#ifdef ATOMIC_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Atomic
{

NETService::NETService(Context* context) :
    Object(context),
    brokerActive_(false)
{
    fd_[0] = INVALID_IPCHANDLE_VALUE;
    fd_[1] = INVALID_IPCHANDLE_VALUE;

    ipc_ = GetSubsystem<IPC>();

    SubscribeToEvent(E_LOGMESSAGE, HANDLER(NETService, HandleLogMessage));
}

NETService::~NETService()
{

}

void NETService::HandleIPCInitialize(StringHash eventType, VariantMap& eventData)
{
    brokerActive_ = true;
}

void NETService::ProcessArguments() {

    const Vector<String>& arguments = GetArguments();

    int id = -1;

    for (unsigned i = 0; i < arguments.Size(); ++i)
    {
        if (arguments[i].Length() > 1)
        {
            String argument = arguments[i].ToLower();
            // String value = i + 1 < arguments.Size() ? arguments[i + 1] : String::EMPTY;

            if (argument.StartsWith("--ipc-id="))
            {
                Vector<String> idc = argument.Split(argument.CString(), '=');
                if (idc.Size() == 2)

                    id = ToInt(idc[1].CString());
            }

            else if (argument.StartsWith("--ipc-server=") || argument.StartsWith("--ipc-client="))
            {
                LOGINFOF("Starting IPCWorker %s", argument.CString());

                Vector<String> ipc = argument.Split(argument.CString(), '=');

                if (ipc.Size() == 2)
                {
                    if (argument.StartsWith("--ipc-server="))
                    {
#ifdef ATOMIC_PLATFORM_WINDOWS
                        // clientRead
                        WString wipc(ipc[1]);
                        HANDLE pipe = reinterpret_cast<HANDLE>(_wtoi64(wipc.CString()));
                        fd_[0] = pipe;
#else
                        int fd = ToInt(ipc[1].CString());
                        fd_[0] = fd;
#endif
                    }
                    else
                    {
#ifdef ATOMIC_PLATFORM_WINDOWS
                        // clientWrite
                        WString wipc(ipc[1]);
                        HANDLE pipe = reinterpret_cast<HANDLE>(_wtoi64(wipc.CString()));
                        fd_[1] = pipe;
#else
                        int fd = ToInt(ipc[1].CString());
                        fd_[1] = fd;
#endif
                    }

                }

            }

        }
    }

    if (id > 0 && fd_[0] != INVALID_IPCHANDLE_VALUE && fd_[1] != INVALID_IPCHANDLE_VALUE)
    {
        SubscribeToEvent(E_IPCINITIALIZE, HANDLER(NETService, HandleIPCInitialize));
        ipc_->InitWorker((unsigned) id, fd_[0], fd_[1]);
    }
}


void NETService::HandleLogMessage(StringHash eventType, VariantMap& eventData)
{
    using namespace LogMessage;

    if (brokerActive_)
    {

        if (ipc_.Null())
            return;

        VariantMap logEvent;
        logEvent[IPCWorkerLog::P_LEVEL] = eventData[P_LEVEL].GetInt();
        logEvent[IPCWorkerLog::P_MESSAGE] = eventData[P_MESSAGE].GetString();
        ipc_->SendEventToBroker(E_IPCWORKERLOG, logEvent);
    }

}


}
