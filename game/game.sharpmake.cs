
using System.IO;
using Sharpmake;
using System;

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


            SourceFilesExtensions.Add(".xml");
            SourceFilesExtensions.Add(".gradle");
            SourceFilesExtensions.Add(".glsl");

            var platformPath = Path.Combine(Globals.RootDirectory, "platform");
            AdditionalSourceRootPaths.Add(platformPath);

            // Show resource files in project
            var projectPath = Path.Combine(Globals.RootDirectory, "projects", Name);
            GenerateCopyAndroidResourcesBatchFile(projectPath);

        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectFileName = "[project.Name]_[target.DevEnv]_[target.Platform]";
            conf.ProjectPath = Path.Combine(Globals.RootDirectory, @"projects\[project.Name]");
            conf.IncludePaths.Add(Path.Combine(Globals.RootDirectory, @"game/src"));


            conf.Options.Add(Options.Agde.Compiler.CppLanguageStandard.Cpp17);

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

            conf.VcxprojUserFile = new Configuration.VcxprojUserFileSettings
            {
                LocalDebuggerWorkingDirectory = "$(TargetDir)",
                LocalDebuggerEnvironment = File.Exists(Path.Combine(Globals.RootDirectory, @"tools\MetaXRSimulator\meta_openxr_simulator.json")) ? "XR_RUNTIME_JSON=" + Path.Combine(Globals.RootDirectory, @"tools\MetaXRSimulator\meta_openxr_simulator.json") : "",
            };

            var realProjectPath = ResolveString(SharpmakeCsProjectPath, conf, target);

            // compile shaders with glslc 
            conf.EventPostBuild.Add($"if not exist $(OutDir)data mkdir $(OutDir)data");
            conf.EventPostBuild.Add($"\"$(VULKAN_SDK)\\Bin\\glslc.exe\" -fshader-stage=vertex -o $(OutDir)data\\VertexShader.spv {realProjectPath}\\data\\VertexShader.glsl");
            conf.EventPostBuild.Add($"\"$(VULKAN_SDK)\\Bin\\glslc.exe\" -fshader-stage=fragment -o $(OutDir)data\\PixelShader.spv {realProjectPath}\\data\\PixelShader.glsl");


            // if not set, no precompile option will be used.
            //conf.PrecompHeader = "pch.h";
            //conf.PrecompSource = "pch.cpp";

            conf.CustomProperties.Add("CustomOptimizationProperty", $"Custom-{target.Optimization}");

            conf.AddPublicDependency<OpenXRProject>(target);
            //conf.AddPublicDependency<EASTLProject>(target);

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
            conf.LibraryFiles.Add("vulkan-1.lib");
            conf.SourceFilesBuildExclude.Add(Path.Combine(Android.GlobalSettings.NdkRoot, @"sources\android\native_app_glue\android_native_app_glue.c"));
        }



        public override void ConfigureAgde(Configuration conf, CommonTarget target)
        {
            base.ConfigureAgde(conf, target);


            conf.IncludePaths.Add(Path.Combine(Android.GlobalSettings.NdkRoot, @"sources\android")); // For android_native_app_glue.h
            conf.AdditionalLinkerOptions.Add("-llog", "-landroid", "-lvulkan");

            conf.TargetPath = Path.Combine(conf.TargetPath, GetABI(target));
            conf.TargetFileName = LowerName;

            // copy android resources before build
            conf.EventPreBuild.Add($"call {Path.Combine(Globals.RootDirectory, @"projects\[project.Name]\copy_resources.bat")}");

        }

        private void GenerateCopyAndroidResourcesBatchFile(string projectPath)
        {
            //string dstPath = Path.Combine(projectPath, @"src\main");
            //if (!Directory.Exists(dstPath))
            //{
            //    Directory.CreateDirectory(dstPath);
            //}
            //string srcManifestFile = Path.Combine(SharpmakeCsProjectPath, @"..\platform\meta\resources\AndroidManifest.xml");
            //string dstManifestFile = Path.Combine(dstPath, "AndroidManifest.xml");
            //Util.ForceCopy(srcManifestFile, dstManifestFile);

            //// Copy module build gradle file to project folder
            //string srcModulePath = Path.Combine(SharpmakeCsProjectPath, @"..\platform\meta\gradle\app");
            //AndroidUtil.DirectoryCopy(srcModulePath, projectPath);

            // translate above code to batch files
            Directory.CreateDirectory(projectPath + "\\src\\main");
            string copyResBat = Path.Combine(projectPath, "copy_res.bat");
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
