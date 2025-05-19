# How to create an iOS build

## Requirements

- You'll need a payable developer account at Apple to deploy the software to your device
- Install Xcode and brew, then install imagemagick and cmake using brew
- For older devices with arm7 (e.g. iPad2) you'll need macos Monterey and Xcode 13.4.1

## Workflow

1. Clone the navit repository

2. cd into the repository

3. Create a build folder

4. cd into build folder

5. For arm64 devices use:

   ```bash
   cmake -G Xcode ../ -DCMAKE_TOOLCHAIN_FILE=../Toolchain/xcode-iphone_new.cmake -DUSE_PLUGINS=0 -DBUILD_MAPTOOL=0 -DSAMPLE_MAP=0 -DXSLTS=iphone -DUSE_UIKIT=1 -DDEVELOPMENT_TEAM_ID="<enter your team id>" -DCODE_SIGN_IDENTITY="iPhone Developer"
   ```

   For iOS device with arm7 use an old SDK and this command:

   ```bash
   cmake -G Xcode ../ -DCMAKE_TOOLCHAIN_FILE=../Toolchain/xcode-iphone_new.cmake -DUSE_PLUGINS=0 -DBUILD_MAPTOOL=0 -DSAMPLE_MAP=0 -DXSLTS=iphone -DUSE_UIKIT=1 -DCMAKE_IOS_SDK_ROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/ Developer/SDKs/iPhoneOS15.5.sdk -DIOS_ARCH=armv7 -DIPHONEOS_DEPLOYMENT_TARGET="9.3"
   ```

   For iOS Simulator use this command:

   ```bash
   cmake -G Xcode ../ -DCMAKE_TOOLCHAIN_FILE=../Toolchain/xcode-iphone_new.cmake -DUSE_PLUGINS=0 -DBUILD_MAPTOOL=1 -DSAMPLE_MAP=1 -DXSLTS=iphone -DUSE_UIKIT=1 -DIOS_PLATFORM=SIMULATOR
   ```
