using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Sharpmake;
using Sharpmake.Generators.FastBuild;


[module: Sharpmake.DebugProjectName("Sharpmake.Que")]
[module: Sharpmake.Include("game/*.sharpmake.cs")]
[module: Sharpmake.Include("tools/*.sharpmake.cs")]

[module: Sharpmake.Include("deps/*.sharpmake.cs")]
[module: Sharpmake.Include("deps/**/*.sharpmake.cs")]

namespace Que
{
    public static class Globals
    {
        // branch root path relative to current sharpmake file location
        public const string RelativeRootPath = @".";
        public static string RootDirectory;
        public static string TmpDirectory { get { return Path.Combine(RootDirectory, "temp"); } }
        public static string OutputDirectory { get { return Path.Combine(TmpDirectory, "bin"); } }
    }

    public static class AndroidUtil
    {
        public static void SetupDefaultSDKPaths()
        {
            string AndroidSdkPath = System.Environment.GetEnvironmentVariable("ANDROID_SDK_ROOT");
            string AndroidNdkPath = System.Environment.GetEnvironmentVariable("ANDROID_NDK_ROOT");
            string JavaPath = System.Environment.GetEnvironmentVariable("JAVA_HOME");

            if (AndroidSdkPath == null)
            {
                throw new Error("ANDROID_SDK_ROOT environment variable undefined");
            }
            else if (!Directory.Exists(AndroidSdkPath))
            {
                throw new Error(AndroidSdkPath + " directory does not exist.");
            }

            if (AndroidNdkPath == null)
            {
                throw new Error("ANDROID_NDK_ROOT environment variable undefined");
            }
            else if (!Directory.Exists(AndroidNdkPath))
            {
                throw new Error(AndroidNdkPath + " directory does not exist.");
            }

            if (JavaPath == null)
            {
                throw new Error("JAVA_HOME environment variable undefined");
            }

            // Android global settings config
            {
                Android.GlobalSettings.AndroidHome = AndroidSdkPath;
                Android.GlobalSettings.NdkRoot = AndroidNdkPath;
                Android.GlobalSettings.JavaHome = JavaPath;
            }
        }

        public static void DirectoryCopy(string sourceDirName, string destDirName)
        {
            if (!Directory.Exists(sourceDirName))
            {
                throw new Error($"Source path does not exist {sourceDirName}");
            }
            // Get the files in the directory and copy them to the new location.
            string[] files = Util.DirectoryGetFiles(sourceDirName);
            foreach (string file in files)
            {
                string srcFile = file;
                string relativePath = Util.PathGetRelative(sourceDirName, srcFile);
                string destFile = Path.Combine(destDirName, relativePath);
                string destDir = new FileInfo(destFile).DirectoryName;
                if (!Directory.Exists(destDir))
                {
                    Directory.CreateDirectory(destDir);
                }
                Util.ForceCopy(srcFile, destFile);
            }
        }
    }

    [Sharpmake.Generate]
    public class QueSolution : Sharpmake.Solution
    {
        public QueSolution() : base(typeof(CommonTarget))
        {
            Name = "Que";

            AddTargets(CommonTarget.GetDefaultTargets());
        }

        private bool _hasCopiedResources = false;

        [ConfigurePriority(ConfigurePriorities.Platform)]
        [Configure(Platform.agde)]
        public  void ConfigureAgde(Configuration conf, CommonTarget target)
        {

            if (!_hasCopiedResources)
            {
                //copy top-level build gradle files to root dir
                AndroidUtil.DirectoryCopy(Path.Combine(conf.Solution.SharpmakeCsPath, @"platform\meta\gradle\root"), conf.SolutionPath);
                _hasCopiedResources = true;

                var gradlePropertiesFile = Path.Combine(conf.SolutionPath, "gradle.properties");
                if (File.Exists(gradlePropertiesFile))
                {
                    using (StreamWriter sw = File.AppendText(gradlePropertiesFile))
                    {
                        sw.WriteLine(string.Format("ndkRoot={0}", Android.GlobalSettings.NdkRoot.Replace("\\", "/")));
                    }
                }
            }
        }

        [ConfigurePriority(ConfigurePriorities.All)]
        [Configure]
        public virtual void ConfigureAll(Configuration conf, CommonTarget target)
        {
            conf.SolutionFileName = "[solution.Name]_[target.DevEnv]_[target.Platform]";
            conf.SolutionPath = @".\projects";
            conf.AddProject<QueProject>(target);
        }
    }


    public static class Main
    {
        private static void ConfigureRootDirectory()
        {
            FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();
            string rootDirectory = Path.Combine(fileInfo.DirectoryName, Globals.RelativeRootPath);
            Globals.RootDirectory = Util.SimplifyPath(rootDirectory);
        }

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            AndroidUtil.SetupDefaultSDKPaths();

            ConfigureRootDirectory();

            KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2022, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.Latest);
            arguments.Generate<QueSolution>();
        }
    }
}