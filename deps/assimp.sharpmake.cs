using System.IO;
using Sharpmake;
using System.Collections.Generic;
using Que;
using System;

[Sharpmake.Export]
public class Assimp : CommonProject
{
    public Assimp()
    {
        Name = "Assimp";
        SourceRootPath = @"[project.SharpmakeCsPath]/assimp";
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, CommonTarget target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "Externals";
        // common for all platforms
        conf.IncludePaths.Add(@"[project.SourceRootPath]\include");

        if (target.Platform == Platform.win64)
        {
            Console.WriteLine("Configuring assimp for Windows");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib\x64\assimp-vc143-mt.lib");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\bin\x64\assimp-vc143-mt.dll");
        }

        if (target.Platform == Platform.agde)
        {
            Console.WriteLine("Configuring assimp for Android");
            conf.LibraryPaths.Add(@"[project.SourceRootPath]\lib\arm64");
            conf.LibraryFiles.Add("libandroid_jniiosystem.a");
            conf.LibraryFiles.Add("assimp.so");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\lib\arm64\libassimp.so");
        }
    }
    public override void ConfigureAgde(Configuration conf, CommonTarget target)
    {
        base.ConfigureAgde(conf, target);
    }
}
