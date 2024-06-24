using System.IO;
using Sharpmake;
using System.Collections.Generic;
using Que;
using System;

[Sharpmake.Export]
public class Fmod : CommonProject
{
    public Fmod()
    {
        Name = "Fmod";
        SourceRootPath = @"[project.SharpmakeCsPath]";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, CommonTarget target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "Externals";
        // common for all platforms

        if (target.Platform == Platform.win64)
        {
            Console.WriteLine("Configuring fmod for Windows");
            conf.IncludePaths.Add(@"[project.SourceRootPath]\win\core\inc");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\win\core\lib\x64\fmod_vc.lib");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\win\core\lib\x64\fmod.dll");
        }

        if (target.Platform == Platform.agde)
        {
            Console.WriteLine("Configuring fmod for Android");

            conf.IncludePaths.Add(@"[project.SourceRootPath]\android\core\inc");
            conf.LibraryPaths.Add(@"[project.SourceRootPath]\android\core\lib\arm64-v8a");
            conf.LibraryFiles.Add("libfmod.so");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\android\core\lib\arm64-v8a\libfmod.so");
        }
    }

    public override void ConfigureAgde(Configuration conf, CommonTarget target)
    {
        base.ConfigureAgde(conf, target);
    }
}
