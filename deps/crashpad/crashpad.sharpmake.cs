using System.IO;
using Sharpmake;
using System.Collections.Generic;
using Que;
using System;

[Sharpmake.Export]
public class Crashpad : CommonProject
{
    public Crashpad()
    {
        Name = "Crashpad";
        SourceRootPath = @"[project.SharpmakeCsPath]";
        AddTargets(CommonTarget.GetDefaultTargets());
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, CommonTarget target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "Externals";
    }

    public override void ConfigureWin64(Configuration conf, CommonTarget target)
    {
        base.ConfigureWin64(conf, target);

        conf.IncludePaths.Add(@"[project.SourceRootPath]\include");
        conf.IncludePaths.Add(@"[project.SourceRootPath]\include\mini_chromium");

        if (target.Optimization == Optimization.Debug)
        {
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt_debug\base.lib");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt_debug\client.lib");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt_debug\common.lib");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt_debug\util.lib");
        }
        else
        {
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt\base.lib");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt\client.lib");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt\common.lib");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib_mt\util.lib");
        }
        conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\bin\crashpad_handler.exe");

        conf.Defines.Add("CRASHPAD_ENABLED");
    }
    public override void ConfigureAgde(Configuration conf, CommonTarget target)
    {
        base.ConfigureAgde(conf, target);
    }

    public override void ConfigureDebug(Configuration conf, CommonTarget target)
    {
        base.ConfigureDebug(conf, target);

    }
}
