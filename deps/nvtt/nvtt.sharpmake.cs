using System.IO;
using Sharpmake;
using System.Collections.Generic;
using Que;
using System;

[Sharpmake.Export]
public class Nvtt : CommonProject
{
    public Nvtt()
    {
        Name = "Nvtt";
        SourceRootPath = @"[project.SharpmakeCsPath]";
    }

    public override void ConfigureWin64(Configuration conf, CommonTarget target)
    {
        base.ConfigureWin64(conf, target);

        conf.SolutionFolder = "Externals";

        if (target.Platform == Platform.win64)
        {
            Console.WriteLine("Configuring fmod for Windows");
            conf.IncludePaths.Add(@"[project.SourceRootPath]\win\include");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\win\lib\x64-v142\nvtt30205.lib");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\win\bin\nvtt30205.dll");
        }
    }
}
