
using System.IO;
using Sharpmake;
using System;
using System.Text;
using System.Net.Http.Headers;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Que
{



    [Sharpmake.Generate]
    public class QueProject : CommonProject
    {
        public QueProject()
        {
            Name = "Game";
            AddTargets(CommonTarget.GetDefaultTargets());


            SourceFiles.Add(Path.Combine(Android.GlobalSettings.NdkRoot, @"sources\android\native_app_glue\android_native_app_glue.c"));
            SourceFiles.Add(Path.Combine(Globals.RootDirectory, @"deps\tracy\public\TracyClient.cpp"));

            SourceFilesExtensions.Add(".xml");
            SourceFilesExtensions.Add(".gradle");
            SourceFilesExtensions.Add(".vert");
            SourceFilesExtensions.Add(".frag");
            SourceFilesExtensions.Add(".glsl");
            SourceFilesExtensions.Add(".hlsl");
            SourceFilesExtensions.Add(".hlsli");
            SourceFilesExtensions.Add(".py");


            var platformPath = Path.Combine(Globals.RootDirectory, "platform");
            AdditionalSourceRootPaths.Add(platformPath);

            // Show resource files in project
            var projectPath = Path.Combine(Globals.RootDirectory, "projects", Name);
            GenerateCopyAndroidResourcesBatchFile(projectPath);

            var configPath = Path.Combine(projectPath, @"config.generated.h");

            if (File.Exists(configPath))
                File.Delete(configPath);

            File.Create(configPath).Close();
            SourceFiles.Add(configPath);
        }

        public string GetConfigFilePath(Configuration conf, CommonTarget target)
        {
            return ResolveString(conf.ProjectPath, conf, target) + "/config.generated.h";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Defines.Add("TRACY_ENABLE");

            conf.ProjectFileName = "[project.Name]_[target.DevEnv]_[target.Platform]";
            conf.ProjectPath = Path.Combine(Globals.RootDirectory, @"projects\[project.Name]");
            conf.IncludePaths.Add(Path.Combine(Globals.RootDirectory, @"game/src"));


            conf.Options.Add(Options.Agde.Compiler.CppLanguageStandard.Cpp17);

            conf.Defines.Add("FMT_HEADER_ONLY=1");



            if (target.Platform == Platform.win64)
            {
                conf.Defines.Add("XR_USE_PLATFORM_WIN32");
                conf.Defines.Add("XR_USE_GRAPHICS_API_VULKAN");
                conf.Defines.Add("XR_OS_WINDOWS");
            }
            else if (target.Platform == Platform.agde)
            {
                conf.Defines.Add("XR_USE_PLATFORM_ANDROID");
                conf.Defines.Add("XR_USE_GRAPHICS_API_VULKAN");
                conf.Defines.Add("XR_OS_ANDROID");
            }

            // glm include
            conf.IncludePaths.Add(Path.Combine(Globals.RootDirectory, @"deps/glm"));

            // vulkan common
            conf.IncludePaths.Add(Environment.GetEnvironmentVariable("VULKAN_SDK") + @"\Include");
            conf.LibraryPaths.Add(Environment.GetEnvironmentVariable("VULKAN_SDK") + @"\Lib");

            // tracy include
            conf.IncludePaths.Add(Path.Combine(Globals.RootDirectory, @"deps/tracy/public"));
            conf.IncludePaths.Add(Path.Combine(Globals.RootDirectory, @"deps"));
            conf.IncludePaths.Add(Path.Combine(Globals.RootDirectory, @"deps/entt/single_include"));

            // copy dxil
            conf.LibraryFiles.Add("dxcompiler.lib");
            var dxcPath = Environment.GetEnvironmentVariable("VULKAN_SDK") + "\\Bin\\dxcompiler.dll";
            conf.TargetCopyFiles.Add(dxcPath);

            conf.VcxprojUserFile = new Configuration.VcxprojUserFileSettings
            {
                LocalDebuggerWorkingDirectory = "$(TargetDir)",
                LocalDebuggerEnvironment = File.Exists(Path.Combine(Globals.RootDirectory, @"tools\MetaXRSimulator\meta_openxr_simulator.json")) ? "XR_RUNTIME_JSON=" + Path.Combine(Globals.RootDirectory, @"tools\MetaXRSimulator\meta_openxr_simulator.json") : "",
            };


            if (target.Optimization == Optimization.Debug)
                conf.Defines.Add("JPH_DEBUG_RENDERER");

            conf.Defines.Add("JPH_PROFILE_ENABLED");

            // if not set, no precompile option will be used.
            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp";

            conf.PrecompSourceExcludeFolders = new Strings("src/lib/", "../deps/tracy/public/");

            conf.CustomProperties.Add("CustomOptimizationProperty", $"Custom-{target.Optimization}");

            conf.AddPublicDependency<OpenXRProject>(target);
            conf.AddPublicDependency<Assimp>(target);
            conf.AddPublicDependency<Jolt>(target);
            conf.AddPublicDependency<SDL>(target);
            conf.AddPublicDependency<Fmod>(target);
            conf.AddPublicDependency<Crashpad>(target);
            conf.AddPublicDependency<Nvtt>(target);
            conf.AddPublicDependency<Ngsdk>(target);

            //conf.AddPublicDependency<EASTLProject>(target);

            // conf.Options.Add(Options.Vc.Compiler.JumboBuild.Enable);
            // conf.MaxFilesPerJumboFile = 0;
            // conf.MinFilesPerJumboFile = 2;
            // conf.MinJumboFiles = 1;

            conf.Output = Configuration.OutputType.Exe;

        }

        public override void PostResolve()
        {
            base.PostResolve();


        }

        public override void ConfigureWin64(Configuration conf, CommonTarget target)
        {
            base.ConfigureWin64(conf, target);

            conf.LibraryPaths.Add(Environment.GetEnvironmentVariable("VULKAN_SDK") + @"\Lib");

            //conf.LibraryFiles.Add("vulkan-1.lib");
            conf.LibraryFiles.Add("Rpcrt4.lib");

            conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);

            conf.SourceFilesBuildExclude.Add(Path.Combine(Android.GlobalSettings.NdkRoot, @"sources\android\native_app_glue\android_native_app_glue.c"));

            var realProjectPath = ResolveString(SharpmakeCsProjectPath, conf, target);

            //conf.EventPostBuild.Add($"if not exist $(OutDir)data mkdir $(OutDir)data");

            var dataPath = Path.Combine(realProjectPath, "data");
            var targetPath = Path.Combine(ResolveString(conf.TargetPath, conf, target), "data");



            if (target.Optimization == Optimization.Release)
                conf.EventPostBuild.Add($"xcopy /s /y {realProjectPath}\\data $(OutDir)\\data");
            else
            {
                //if (!Util.CreateSymbolicLink(targetPath, dataPath, true))
                //    Debug.Assert(false);

                conf.VcxprojUserFile.LocalDebuggerCommandArguments = "sc";
            }

            // shader pipeline
            //GenerateShaderCompileBat($"{realProjectPath}\\data\\shader", ResolveString(conf.TargetPath, conf, target) + "\\shader");
            //conf.EventPreBuild.Add($"call  {conf.TargetPath + "\\shader\\shader_compile.bat"}");
            conf.EventPreBuild.Add($"python {realProjectPath + "\\scripts\\compile_shaders.py"} {realProjectPath + "\\data\\shader\\hlsl"} {conf.TargetPath + "\\shader"}");

            conf.Defines.Add("JPH_FLOATING_POINT_EXCEPTIONS_ENABLED");
            conf.Defines.Add("GLM_FORCE_DEPTH_ZERO_TO_ONE");
            if (target.Optimization == Optimization.Debug)
                CheckForLivePPSupport(conf, target);
        }

        private void CheckForLivePPSupport(Configuration conf, CommonTarget target)
        {
            if (Directory.Exists("deps/LivePP"))
            {
                Console.WriteLine("Found Live++, enabling support");
                conf.Defines.Add("LIVEPP_ENABLED");
                File.AppendAllText(GetConfigFilePath(conf, target), $"#define LIVEPP_PATH L\"{Path.GetFullPath("deps/LivePP").Replace(@"\", @"\\")}\"");
            }
        }

        public override void ConfigureAgde(Configuration conf, CommonTarget target)
        {
            base.ConfigureAgde(conf, target);


            conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);

            conf.IncludePaths.Add(Path.Combine(Android.GlobalSettings.NdkRoot, @"sources\android")); // For android_native_app_glue.h
            conf.AdditionalLinkerOptions.Add("-llog", "-landroid", "-lvulkan");


            conf.LibraryFiles.Add(Path.Combine(Globals.RootDirectory, @"deps\vulkan_android_validation_layer\libVkLayer_khronos_validation.so"));
            conf.TargetCopyFiles.Add(Path.Combine(Globals.RootDirectory, @"deps\vulkan_android_validation_layer\libVkLayer_khronos_validation.so"));
            conf.LibraryPaths.Add(Path.Combine(Globals.RootDirectory, @"deps\vulkan_android_validation_layer"));

            conf.TargetPath = Path.Combine(conf.TargetPath, GetABI(target));
            conf.TargetFileName = LowerName;

            // copy android resources before build
            conf.EventPreBuild.Add($"call {Path.Combine(Globals.RootDirectory, @"projects\[project.Name]\copy_resources.bat")}");

            var realProjectPath = ResolveString(SharpmakeCsProjectPath, conf, target);

            var androidAssetsPath = ResolveString(Path.Combine(Globals.RootDirectory, @"projects\[project.Name]\src\main\assets\"), conf, target);

            // copy assets
            conf.EventPostBuild.Add($"xcopy /s /y {realProjectPath}\\data {androidAssetsPath}\\data");

            // compile shaders with glslc 
            conf.EventPostBuild.Add($"if not exist {androidAssetsPath}data mkdir {androidAssetsPath}data");
            GenerateShaderCompileBat($"{realProjectPath}\\data\\shader", androidAssetsPath + "\\data\\shader");

            conf.EventPreBuild.Add($"call  {androidAssetsPath + "\\data\\shader\\shader_compile.bat"}");

        }

        private void GenerateShaderCompileBat(string srcPath, string targetPath)
        {
            // loop over files in srcPath and run glslc on them
            StringBuilder batchContent = new StringBuilder();
            batchContent.AppendLine("@echo off");

            string vulkanPath = Environment.GetEnvironmentVariable("VULKAN_SDK");

            foreach (string file in Directory.GetFiles(srcPath, "*.vert"))
            {
                batchContent.AppendLine($"\"{vulkanPath}\\Bin\\glslc.exe\" -o {targetPath}\\{Path.GetFileName(file)}.spv {file}");
            }

            foreach (string file in Directory.GetFiles(srcPath, "*.frag"))
            {
                batchContent.AppendLine($"\"{vulkanPath}\\Bin\\glslc.exe\" -o {targetPath}\\{Path.GetFileName(file)}.spv {file}");
            }

            if (!Directory.Exists(targetPath))
                Directory.CreateDirectory(targetPath);

            File.WriteAllText(targetPath + "\\shader_compile.bat", batchContent.ToString());
        }

        private void GenerateCopyAndroidResourcesBatchFile(string projectPath)
        {
            Directory.CreateDirectory(projectPath + "\\src\\main");

            string copyResBat = Path.Combine(projectPath, "copy_resources.bat");
            string copyResCmd = $"xcopy /E /Y /I {SharpmakeCsProjectPath}\\..\\platform\\meta\\resources\\AndroidManifest.xml {projectPath}\\src\\main";

            File.WriteAllText(copyResBat, copyResCmd);
            File.AppendAllText(copyResBat, "\n");

            string copyGradeCmd = $"xcopy /E /Y /I {SharpmakeCsProjectPath}\\..\\platform\\meta\\gradle\\app {projectPath}";
            File.AppendAllText(copyResBat, copyGradeCmd);
        }

        private string GetABI(CommonTarget target)
        {
            switch (target.AndroidBuildTargets)
            {
                case Android.AndroidBuildTargets.arm64_v8a:
                    return "arm64-v8a";
                case Android.AndroidBuildTargets.x86_64:
                    return "x86_64";
                case Android.AndroidBuildTargets.armeabi_v7a:
                    return "armeabi-v7a";
                case Android.AndroidBuildTargets.x86:
                    return "x86";
                default:
                    throw new System.Exception($"Something wrong? {target.AndroidBuildTargets} is not supported Android target for AGDE.");
            }
        }
    }
}
