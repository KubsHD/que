using System.IO;
using Sharpmake;
using System.Collections.Generic;
using Que;
using System;

[Sharpmake.Export]
public class Jolt : CommonProject
{
    public Jolt()
    {
        Name = "Jolt";
        SourceRootPath = @"[project.SharpmakeCsPath]/jolt";
        AddTargets(CommonTarget.GetDefaultTargets());
    }

    [Configure]
    public override void ConfigureAll(Configuration conf, CommonTarget target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "Externals";

        // common for all platforms
        conf.IncludePaths.Add(@"[project.SourceRootPath]");
    }

    public override void ConfigureWin64(Configuration conf, CommonTarget target)
    {
        base.ConfigureWin64(conf, target);

        // todo: add automatic library compilation

        Console.WriteLine("Configuring jolt for Windows");
        conf.LibraryFiles.Add(@$"[project.SourceRootPath]\Build\VS2022_CL\{target.Optimization}\Jolt.lib");
    }
    public override void ConfigureAgde(Configuration conf, CommonTarget target)
    {
        base.ConfigureAgde(conf, target);

        // todo

        conf.LibraryFiles.Add(@$"[project.SourceRootPath]\Build\Android\UnitTests\.cxx\{((target.Optimization == Optimization.Debug) ? target.Optimization : "RelWithDebInfo")}\arm64-v8a\libjolt.a");

    }

    public override void ConfigureDebug(Configuration conf, CommonTarget target)
    {
        base.ConfigureDebug(conf, target);

        // https://github.com/jrouwe/JoltPhysics/issues/987
        conf.Defines.Add("JPH_DEBUG_RENDERER");
    }
}
