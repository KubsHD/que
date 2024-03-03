using Sharpmake;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Que
{
    using AndroidBuildTargets = Sharpmake.Android.AndroidBuildTargets;


    [DebuggerDisplay("\"{Platform}_{DevEnv}\" {Name}")]
    public class CommonTarget : Sharpmake.ITarget
    {
        public Platform Platform;
        public DevEnv DevEnv;
        public Optimization Optimization;
        public Blob Blob;
        public AndroidBuildTargets AndroidBuildTargets = AndroidBuildTargets.arm64_v8a;
        public CommonTarget() { }

        public CommonTarget(
            Platform platform,
            DevEnv devEnv,
            Optimization optimization,
            Blob blob
        )
        {
            Platform = platform;
            DevEnv = devEnv;
            Optimization = optimization;
            Blob = blob;
        }

        // this is the output directory in vs project 
        public override string Name
        {
            get
            {
                var nameParts = new List<string>();

                nameParts.Add(Optimization.ToString());

                return string.Join("_", nameParts);
            }
        }

        public string SolutionPlatformName
        {
            get
            {
                return Platform.ToString() + "_" + this.AndroidBuildTargets.ToString();
            }
        }

        /// <summary>
        /// returns a string usable as a directory name, to use for instance for the intermediate path
        /// </summary>
        public string DirectoryName
        {
            get
            {
                var dirNameParts = new List<string>();

                dirNameParts.Add(Platform.ToString());
                dirNameParts.Add(Optimization.ToString());

                if (Platform == Platform.agde)
                    dirNameParts.Add(AndroidBuildTargets.ToString());

                return string.Join("_", dirNameParts);
            }
        }

        public override Sharpmake.Optimization GetOptimization()
        {
            switch (Optimization)
            {
                case Optimization.Debug:
                    return Sharpmake.Optimization.Debug;
                case Optimization.Release:
                    return Sharpmake.Optimization.Release;
                default:
                    throw new NotSupportedException("Optimization value " + Optimization.ToString());
            }
        }

        public override Platform GetPlatform()
        {
            return Platform;
        }

        public static CommonTarget[] GetDefaultTargets()
        {
            var result = new List<CommonTarget>();
            result.Add(new CommonTarget(
                Platform.agde,
                DevEnv.vs2022,
                Optimization.Debug | Optimization.Release,
                Blob.NoBlob
            ));
            result.Add(new CommonTarget(Platform.win64, DevEnv.vs2022, Optimization.Debug | Optimization.Release, Blob.NoBlob));
            return result.ToArray();
        }

        public static CommonTarget GetWin64Target()
        {
            return new CommonTarget(Platform.win64, DevEnv.vs2022, Optimization.Debug | Optimization.Release, Blob.NoBlob);
        }

        public static CommonTarget[] GetAndroidTargets()
        {
            var defaultTarget = new CommonTarget(
                Platform.agde,
                DevEnv.vs2022,
                Optimization.Debug | Optimization.Release,
                Blob.NoBlob
            );

            return new[] { defaultTarget };
        }
    }
}
