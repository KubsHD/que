using System.IO;
using Sharpmake;
using System.Collections.Generic;
using Que;
using System;

[Sharpmake.Export]
public class Ngsdk : CommonProject
{
    public Ngsdk()
    {
        Name = "Ngsdk";
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
            Console.WriteLine("Configuring ngfx for Windows");
            conf.IncludePaths.Add(@"[project.SourceRootPath]\include");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib\x64\NGFX_Injection.lib");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\lib\x64\NGFX_Injection.dll");
        }
    }

    public override void ConfigureAgde(Configuration conf, CommonTarget target)
    {
        base.ConfigureAgde(conf, target);
    }
}
