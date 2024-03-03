using System.IO;
using Sharpmake;
using System;

namespace Que
{
    [Sharpmake.Export]
    public class OpenXRProject : CommonProject
    {
        public OpenXRProject()
        {
            Name = "OpenXR";
            SourceRootPath = @"[project.RootPath]";
            AddTargets(CommonTarget.GetDefaultTargets());

            SourceRootPath = Util.GetCurrentSharpmakeFileInfo().DirectoryName;
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            // common for all platforms
            conf.IncludePaths.Add(@"[project.SourceRootPath]\include");

            if (target.Platform == Platform.win64)
            {
                Console.WriteLine("Configuring OpenXR for Windows");
                conf.LibraryFiles.Add(@"[project.SourceRootPath]\windows\lib\openxr_loader.lib");
                conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\windows\bin\openxr_loader.dll");
            }
            else if (target.Platform == Platform.agde)
            {
                Console.WriteLine("Configuring OpenXR for Android");
                if (target.Optimization == Optimization.Debug)
                {
                    conf.LibraryPaths.Add(@"[project.SourceRootPath]\meta\libs\Android\arm64-v8a\Debug");
                    conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\meta\libs\Android\arm64-v8a\Debug\libopenxr_loader.so");
                }
                else
                {
                    conf.LibraryPaths.Add(@"[project.SourceRootPath]\meta\libs\Android\arm64-v8a\Release");
                    conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\meta\libs\Android\arm64-v8a\Release\libopenxr_loader.so");
                }

                conf.LibraryFiles.Add("libopenxr_loader.so");

            }
        }
    }
}
