## Requirements

- .net (at least 6.0)
- Visual Studio 2022
- [AGDE extension for Visual Studio](https://developer.android.com/games/agde)
- Java 17
- Android Sdk with NDK
- Vulkan SDK

## How to build

1. Set ENV variables:

- `ANDROID_SDK_ROOT` - path to Android SDK
- `ANDROID_NDK_ROOT` - path to Android NDK
- `JAVA_HOME` - path to Java JDK

2. Download MetaXRSimulator from [their developer portal](https://developer.oculus.com/downloads/package/meta-xr-simulator) and unzip it to `tools/MetaXRSimulator` folder.

3. Run `GenerateSolutions.bat`

4. Open solution:
   - `que_vs2022_agde.sln` for running on Quest
   - `que_vs2022_win64.sln` for running on Windows

## Additional resources

- [Sky cubemap generation tool](https://matheowis.github.io/HDRI-to-CubeMap/)
