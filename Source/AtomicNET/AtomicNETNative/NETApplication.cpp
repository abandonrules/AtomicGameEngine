//
// Copyright (c) 2008-2014 the Urho3D project.
// Copyright (c) 2014-2015, THUNDERBEAST GAMES LLC All rights reserved
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

#include <Atomic/Atomic.h>
#include <Atomic/Engine/Engine.h>
#include <Atomic/Engine/EngineConfig.h>
#include <Atomic/IO/FileSystem.h>
#include <Atomic/IO/Log.h>
#include <Atomic/IO/IOEvents.h>
#include <Atomic/Input/InputEvents.h>
#include <Atomic/Core/Main.h>
#include <Atomic/Core/ProcessUtils.h>
#include <Atomic/Resource/ResourceCache.h>
#include <Atomic/Resource/ResourceEvents.h>
#include <AtomicPlayer/Player.h>
#include <Atomic/DebugNew.h>

#include <Atomic/IPC/IPC.h>

// Move me
#include <Atomic/Environment/Environment.h>

#include "NETCore.h"
#include "NETApplication.h"
#include "NETService.h"

#ifdef __APPLE__
#include <unistd.h>
#endif

namespace Atomic
{

NETApplication::NETApplication(Context* context, bool headless) :
    Application(context),
    headless_(headless)
{
}

void NETApplication::Setup()
{

    if (headless_)
    {
        engineParameters_["Headless"] = true;

        // Register IPC system
        context_->RegisterSubsystem(new IPC(context_));
        context_->RegisterSubsystem(new NETService(context_));
    }

#ifdef ATOMIC_3D
    RegisterEnvironmentLibrary(context_);
#endif

    FileSystem* filesystem = GetSubsystem<FileSystem>();

    engineParameters_.InsertNew("WindowTitle", "AtomicPlayer");

#if (ATOMIC_PLATFORM_ANDROID)
    engineParameters_.InsertNew("FullScreen", true);
    engineParameters_.InsertNew("ResourcePaths", "CoreData;PlayerData;Cache;AtomicResources");
#elif ATOMIC_PLATFORM_WEB
    engineParameters_.InsertNew("FullScreen", false);
    engineParameters_.InsertNew("ResourcePaths", "AtomicResources");
    // engineParameters_.InsertNew("WindowWidth", 1280);
    // engineParameters_.InsertNew("WindowHeight", 720);
#elif ATOMIC_PLATFORM_IOS
    engineParameters_.InsertNew("FullScreen", false);
    engineParameters_.InsertNew("ResourcePaths", "AtomicResources");
#else
    engineParameters_.InsertNew("FullScreen", false);
    engineParameters_.InsertNew("WindowWidth", 1280);
    engineParameters_.InsertNew("WindowHeight", 720);
    //engineParameters_.InsertNew("ResourcePaths", "AtomicResources");
#endif

#if ATOMIC_PLATFORM_WINDOWS || ATOMIC_PLATFORM_LINUX

    engineParameters_.InsertNew("WindowIcon", "Images/AtomicLogo32.png");
    engineParameters_.InsertNew("ResourcePrefixPath", "AtomicPlayer_Resources");

#elif ATOMIC_PLATFORM_ANDROID
    //engineParameters_.InsertNew("ResourcePrefixPath"], "assets");
#elif ATOMIC_PLATFORM_OSX
    engineParameters_.InsertNew("ResourcePrefixPath", "../Resources");
#endif

    const Vector<String>& arguments = GetArguments();

    for (unsigned i = 0; i < arguments.Size(); ++i)
    {
        if (arguments[i].Length() > 1)
        {
            String argument = arguments[i].ToLower();
            String value = i + 1 < arguments.Size() ? arguments[i + 1] : String::EMPTY;

            if (argument == "--log-std")
            {
                SubscribeToEvent(E_LOGMESSAGE, HANDLER(NETApplication, HandleLogMessage));
            }
        }
    }

    // FIXME AtomicNET:
    engineParameters_["ResourcePrefixPath"] = "/Users/josh/Dev/atomic/AtomicGameEngine/Resources/";

    // Use the script file name as the base name for the log file
    engineParameters_.InsertNew("LogName", filesystem->GetAppPreferencesDir("AtomicPlayer", "Logs") + "AtomicPlayer.log");
}

void NETApplication::Start()
{
    Application::Start();

    // Instantiate and register the Player subsystem.

    if (!engineParameters_["Headless"].GetBool())
        context_->RegisterSubsystem(new AtomicPlayer::Player(context_));

    return;
}

int NETApplication::Initialize()
{
    Setup();
    if (exitCode_)
        return exitCode_;

    if (!engine_->Initialize(engineParameters_))
    {
        ErrorExit();
        return exitCode_;
    }

    Start();
    if (exitCode_)
        return exitCode_;

    return 0;
}

bool NETApplication::RunFrame()
{
    if (engine_->IsExiting())
        return false;

    engine_->RunFrame();

    return true;
}

void NETApplication::Shutdown()
{
    Stop();

}

void NETApplication::Stop()
{
    Application::Stop();
}

NETApplication* NETApplication::CreateInternal(bool headless)
{
    return new NETApplication(NETCore::GetContext(), headless);
}

void NETApplication::HandleLogMessage(StringHash eventType, VariantMap& eventData)
{
    using namespace LogMessage;

    int level = eventData[P_LEVEL].GetInt();
    // The message may be multi-line, so split to rows in that case
    Vector<String> rows = eventData[P_MESSAGE].GetString().Split('\n');

    for (unsigned i = 0; i < rows.Size(); ++i)
    {
        if (level == LOG_ERROR)
        {
            fprintf(stderr, "%s\n", rows[i].CString());
        }
        else
        {
            fprintf(stdout, "%s\n", rows[i].CString());
        }
    }

}

}
