using System.IO;
using Sharpmake;
using System.Collections.Generic;
using Que;
using System;

[Sharpmake.Export]
public class SDL : CommonProject
{
    public SDL()
    {
        Name = "SDL";
        SourceRootPath = @"[project.SharpmakeCsPath]/sdl";
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
            Console.WriteLine("Configuring sdl for Windows");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib\x64\SDL3.lib");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\lib\x64\SDL3.dll");
        }
    }
}
