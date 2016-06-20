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

#include <Atomic/IO/File.h>
#include <Atomic/IO/FileSystem.h>

#include "../JSBind.h"
#include "../JSBPackage.h"
#include "../JSBModule.h"
#include "../JSBEnum.h"
#include "../JSBClass.h"
#include "../JSBFunction.h"

#include "CSTypeHelper.h"
#include "CSClassWriter.h"
#include "CSModuleWriter.h"

namespace ToolCore
{

CSModuleWriter::CSModuleWriter(JSBModule *module) : JSBModuleWriter(module)
{

}

void CSModuleWriter::WriteIncludes(String& source)
{

    Vector<String>& includes = module_->includes_;
    for (unsigned i = 0; i < includes.Size(); i++)
    {
        if (includes[i].StartsWith("<"))
            source.AppendWithFormat("#include %s\n", includes[i].CString());
        else
            source.AppendWithFormat("#include \"%s\"\n", includes[i].CString());
    }

    Vector<JSBHeader*> allheaders;

    HashMap<StringHash, SharedPtr<JSBEnum> >::Iterator eitr = module_->enums_.Begin();
    while (eitr != module_->enums_.End())
    {
        allheaders.Push(eitr->second_->GetHeader());
        eitr++;
    }

    HashMap<StringHash, SharedPtr<JSBClass> >::Iterator citr = module_->classes_.Begin();
    while (citr != module_->classes_.End())
    {
        allheaders.Push(citr->second_->GetHeader());
        citr++;
    }

    Vector<JSBHeader*> included;

    for (unsigned i = 0; i < allheaders.Size(); i++)
    {
        JSBHeader* header = allheaders.At(i);

        if (included.Contains(header))
            continue;

        String headerPath = GetPath(header->GetFilePath());

        String headerfile = GetFileNameAndExtension(header->GetFilePath());

        JSBind* jsbind = header->GetSubsystem<JSBind>();

        headerPath.Replace(jsbind->GetSourceRootFolder() + "Source/", "");

        source.AppendWithFormat("#include <%s%s>\n", headerPath.CString(), headerfile.CString());

        included.Push(header);
    }

    source += ToString("\n#include \"CSPackage%s.h\"\n", module_->GetPackage()->GetName().CString());

}

void CSModuleWriter::GenerateNativeSource()
{
    String source = "// This file was autogenerated by JSBind, changes will be lost\n";

    source += "#ifdef ATOMIC_PLATFORM_WINDOWS\n";

    source += "#pragma warning(disable: 4244) // possible loss of data\n";

    source += "#define ATOMIC_EXPORT_API __declspec(dllexport)\n";

    source += "#else\n";

    source += "#define ATOMIC_EXPORT_API\n";

    source += "#endif\n";

    if (module_->Requires("3D"))
    {
        source += "#ifdef ATOMIC_3D\n";
    }

    WriteIncludes(source);

    source += "\n#include <AtomicNET/AtomicNETNative/NETCore.h>\n";

    String ns = module_->GetPackage()->GetNamespace();

    source += "\n\nusing namespace " + ns + ";\n\n";

    source += "\n\nextern \"C\" \n{\n \n";

    source += "// Begin Class Declarations\n";

    source += "// End Class Declarations\n\n";

    source += "// Begin Classes\n";

    Vector<SharedPtr<JSBClass>> classes = module_->classes_.Values();

    for (unsigned i = 0; i < classes.Size(); i++)
    {
        CSClassWriter clsWriter(classes[i]);
        clsWriter.GenerateNativeSource(source);
    }

    source += "// End Classes\n\n";

    // end Atomic namespace
    source += "\n}\n";

    if (module_->Requires("3D"))
    {
        source += "#endif //ATOMIC_3D\n";
    }

    JSBind* jsbind = module_->GetSubsystem<JSBind>();

    String filepath = jsbind->GetDestNativeFolder() + "/CSModule" + module_->name_ + ".cpp";

    File file(module_->GetContext());
    file.Open(filepath, FILE_WRITE);
    file.Write(source.CString(), source.Length());
    file.Close();

}

String CSModuleWriter::GetManagedPrimitiveType(JSBPrimitiveType* ptype)
{
    if (ptype->kind_ == JSBPrimitiveType::Bool)
        return "bool";
    if (ptype->kind_ == JSBPrimitiveType::Int && ptype->isUnsigned_)
        return "uint";
    else if (ptype->kind_ == JSBPrimitiveType::Int)
        return "int";
    if (ptype->kind_ == JSBPrimitiveType::Float)
        return "float";

    return "int";
}

void CSModuleWriter::GenerateManagedClasses(String& source)
{

    Vector<SharedPtr<JSBClass>> classes = module_->classes_.Values();

    for (unsigned i = 0; i < classes.Size(); i++)
    {
        JSBClass* klass = classes.At(i);

        if (klass->IsNumberArray())
            continue;

        CSClassWriter clsWriter(klass);
        clsWriter.GenerateManagedSource(source);

    }

}

void CSModuleWriter::GenerateManagedEnumsAndConstants(String& source)
{
    Vector<SharedPtr<JSBEnum>> enums = module_->enums_.Values();

    Indent();

    for (unsigned i = 0; i < enums.Size(); i++)
    {
        JSBEnum* jenum = enums[i];

        source += "\n";
        String line = "public enum " + jenum->GetName() + "\n";
        source += IndentLine(line);
        source += IndentLine("{\n");

        HashMap<String, String>& values = jenum->GetValues();

        HashMap<String, String>::ConstIterator itr = values.Begin();

        Indent();

        while (itr != values.End())
        {
            String name = (*itr).first_;
            String value = (*itr).second_;

            if (value.Length())
            {
                line = name + " = " + value;
            }
            else
            {
                line = name;
            }

            itr++;

            if (itr != values.End())
                line += ",";

            line += "\n";

            source += IndentLine(line);

        }

        Dedent();

        source += IndentLine("}\n");

    }

    // constants

    HashMap<String, JSBModule::Constant>& constants = module_->GetConstants();

    if (constants.Size())
    {

        source += "\n";

        String line = "public static partial class Constants\n";
        source += IndentLine(line);
        source += IndentLine("{\n");

        const Vector<String>& constantsName = constants.Keys();

        Indent();

        for (unsigned i = 0; i < constantsName.Size(); i++)
        {
            const String& cname = constantsName.At(i);

            JSBModule::Constant& constant = constants[cname];

            String managedType = GetManagedPrimitiveType(constant.type);

            String value = constant.value;

            if (!value.Length())
                continue;

            //static const unsigned M_MIN_UNSIGNED = 0x00000000;
//            /static const unsigned M_MAX_UNSIGNED = 0xffffffff;

            if (cname == "M_MIN_INT")
                value = "int.MinValue";

            if (cname == "M_INFINITY")
                value = "float.MaxValue";

            if (value == "M_MAX_UNSIGNED")
                value = "0xffffffff";

            // Input stuff

            if (module_->GetName() == "Input")
            {
                if (cname.StartsWith("KEY_"))
                {
                    if (value.Length() == 1 && (IsAlpha(value[0]) || IsDigit(value[0])))
                        value = "'" + value + "'";
                }

                // https://raw.githubusercontent.com/flibitijibibo/SDL2-CS/master/src/SDL2.cs

                if (value.StartsWith("SDL_BUTTON_") || value.StartsWith("SDL_HAT_"))
                {
                    value = "(int) SDL." + value;
                }
                else if (value.StartsWith("SDLK_"))
                {
                    value = "(int) SDL.SDL_Keycode." + value;
                }
                else if (value.StartsWith("SDL_SCANCODE_"))
                {
                    value = "(int) SDL.SDL_Scancode." + value;
                }
                else if (value.StartsWith("SDL_CONTROLLER_BUTTON_"))
                {
                    value = "(int) SDL.SDL_GameControllerButton." + value;
                }
                else if (value.StartsWith("SDL_CONTROLLER_AXIS_"))
                {
                    value = "(int) SDL.SDL_GameControllerAxis." + value;
                }
            }

            String line = "public const " + managedType + " " + cname + " = " + value;

            if (managedType == "float" && !line.EndsWith("f") && IsDigit(line[line.Length()-1]))
                line += "f";

            line += ";\n";

            source += IndentLine(line);

        }

        Dedent();

        source += "\n";
        line = "}\n";
        source += IndentLine(line);

    }

    source += "\n";

    Dedent();

}

void CSModuleWriter::GenerateManagedModuleClass(String& sourceOut)
{
    Indent();

    String source;
    String line = ToString("public static partial class %sModule\n", module_->GetName().CString());

    source += IndentLine(line);

    source += IndentLine("{\n");

    Indent();

    source += IndentLine("public static void Initialize()\n");

    source += IndentLine("{\n");

    Indent();

    Vector<SharedPtr<JSBClass>> classes = module_->classes_.Values();

    for (unsigned i = 0; i < classes.Size(); i++)
    {
        JSBClass* klass = classes.At(i);
        JSBPackage* package = module_->GetPackage();

        if (klass->IsNumberArray() || klass->IsAbstract())
            continue;

        line = ToString("NativeCore.RegisterNativeType(typeof(%s));\n", klass->GetName().CString());

        source += IndentLine(line);

        line = ToString("NativeCore.nativeClassIDToManagedConstructor [ %s.csb_%s_%s_GetClassIDStatic ()] = (IntPtr x) => {\n",
                        klass->GetName().CString(), package->GetName().CString(), klass->GetName().CString());

        source += IndentLine(line);

        Indent();

        source += IndentLine(ToString("return new %s (x);\n", klass->GetName().CString()));

        Dedent();

        source += IndentLine("};\n");

    }

    Dedent();

    source += IndentLine("}\n");

    Dedent();

    source += IndentLine("}\n");

    sourceOut += source;

    Dedent();
}

void CSModuleWriter::GenerateManagedSource()
{
    String source = "// Autogenerated";

    String moduleName = module_->GetPackage()->GetNamespace();

    source += "\nusing System;\nusing System.Collections.Generic;\nusing System.Runtime.InteropServices;\n";

    if (moduleName == "Atomic")
        moduleName = "AtomicEngine";

    if (moduleName != "AtomicEngine")
    {
        source += "using AtomicEngine;\n";
    }

    source += "\n\n";

    source += "namespace " + moduleName + "\n";
    source += "{\n";

    GenerateManagedEnumsAndConstants(source);
    GenerateManagedModuleClass(source);
    GenerateManagedClasses(source);

    source += "}\n";


    JSBind* jsbind = module_->GetSubsystem<JSBind>();
    String filepath = jsbind->GetDestScriptFolder() + "/CSModule" + module_->name_ + ".cs";

    File file(module_->GetContext());
    file.Open(filepath, FILE_WRITE);
    file.Write(source.CString(), source.Length());
    file.Close();

}

void CSModuleWriter::GenerateSource()
{
    GenerateNativeSource();
    GenerateManagedSource();
}

}
