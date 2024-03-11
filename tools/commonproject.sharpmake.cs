

using System.IO;
using System.Linq;
using Sharpmake;

namespace Que
{
    public static class ConfigurePriorities
    {
        public const int All = -75;
        public const int Platform = -50;
        public const int Optimization = -25;
        /*     SHARPMAKE DEFAULT IS 0     */
        public const int Blobbing = 10;
        public const int BuildSystem = 30;
    }

    public abstract class CommonProject : Sharpmake.Project
    {
        static public readonly Sharpmake.Options.Android.General.AndroidAPILevel AndroidMinSdkVersion = Sharpmake.Options.Android.General.AndroidAPILevel.Android29;

        protected CommonProject()
            : base(typeof(CommonTarget))
        {
            RootPath = Globals.RootDirectory;
            IsFileNameToLower = false;
            IsTargetFileNameToLower = false;

            AddTargets(CommonTarget.GetDefaultTargets());

            SourceRootPath = @"[project.RootPath]\[project.Name]";
        }

        [ConfigurePriority(ConfigurePriorities.All)]
        [Configure]
        public virtual void ConfigureAll(Configuration conf, CommonTarget target)
        {
            conf.ProjectFileName = "[project.Name]_[target.DevEnv]_[target.Platform]";
            conf.ProjectPath = Path.Combine(Globals.RootDirectory, @"projects\[project.Name]");

            conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);

            conf.Defines.Add("_HAS_EXCEPTIONS=0");

            return;

            conf.ProjectFileName = "[project.Name]_[target.Platform]";
            if (target.DevEnv != DevEnv.xcode)
                conf.ProjectFileName += "_[target.DevEnv]";
            conf.ProjectPath = Path.Combine(Globals.TmpDirectory, @"projects\[project.Name]");
            conf.IntermediatePath = Path.Combine(Globals.TmpDirectory, @"obj\[target.DirectoryName]\[project.Name]");
            conf.TargetPath = Path.Combine(Globals.OutputDirectory, "[target.DirectoryName]");

            conf.TargetLibraryPath = conf.IntermediatePath; // .lib files must be with the .obj files when running in fastbuild distributed mode or we'll have missing symbols due to merging of the .pdb

            conf.TargetFileName += "_" + target.Optimization.ToString().ToLowerInvariant().First(); // suffix with lowered first letter of optim
            if (conf.IsFastBuild)
                conf.TargetFileName += "x";

            conf.Output = Configuration.OutputType.Lib; // defaults to creating static libs
        }

        ////////////////////////////////////////////////////////////////////////
        #region Platforms
        [ConfigurePriority(ConfigurePriorities.Platform)]
        [Configure(Platform.agde)]
        public virtual void ConfigureAgde(Configuration conf, CommonTarget target)
        {
            conf.Options.Add(new Sharpmake.Options.Agde.General.AndroidGradleBuildDir("$(SolutionDir)"));
            conf.Options.Add(AndroidMinSdkVersion); // This will set AndroidMinSdkVersion when project is for AGDE
            conf.Options.Add(Sharpmake.Options.Agde.General.UseOfStl.LibCpp_Shared);
            conf.Options.Add(Sharpmake.Options.Agde.Linker.BuildId.Md5); // This allow to match debug-stripped binaries with their original binary, similar to how ".pdb" work on Windows.

            conf.Options.Add(Options.Vc.Linker.SubSystem.Console);
        }

        [ConfigurePriority(ConfigurePriorities.Platform)]
        [Configure(Platform.win64)]
        public virtual void ConfigureWin64(Configuration conf, CommonTarget target)
        {
            conf.Options.Add(Options.Vc.Linker.SubSystem.Windows);
        }
        #endregion
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        #region Optimizations
        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Debug)]
        public virtual void ConfigureDebug(Configuration conf, CommonTarget target)
        {
            conf.DefaultOption = Options.DefaultTarget.Debug;
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Release)]
        public virtual void ConfigureRelease(Configuration conf, CommonTarget target)
        {
            conf.DefaultOption = Options.DefaultTarget.Release;
        }
        #endregion
        ////////////////////////////////////////////////////////////////////////
    }
}