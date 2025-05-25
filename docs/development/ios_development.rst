==================
How to create an iOS build
==================

Requirements
############
- You'll need a payable developer account at Apple to deploy the software to your device
- Create an AppID and a provisioning profile
- For older devices with arm7 (e.g. iPad2) you'll need macos Monterey and Xcode 13.4.1
- Install Xcode and brew, then install librsvg, saxon and cmake using brew


Workflow
########

1. Clone the navit repository

2. cd into the repository

3. Create a build folder

4. cd into build folder

5. For arm64 devices use:

   ``cmake -G Xcode ../ -DCMAKE_TOOLCHAIN_FILE=../Toolchain/xcode-iphone_new.cmake -DUSE_PLUGINS=0 -DBUILD_MAPTOOL=0 -DSAMPLE_MAP=1 -DXSLTS=iphone -DUSE_UIKIT=1``

   For iOS device with arm7 use an old SDK and this command:

   ``cmake -G Xcode ../ -DCMAKE_TOOLCHAIN_FILE=../Toolchain/xcode-iphone_new.cmake -DUSE_PLUGINS=0 -DBUILD_MAPTOOL=0 -DSAMPLE_MAP=1 -DXSLTS=iphone -DUSE_UIKIT=1 -DCMAKE_IOS_SDK_ROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/ Developer/SDKs/iPhoneOS15.5.sdk -DIOS_ARCH=armv7 -DIPHONEOS_DEPLOYMENT_TARGET="9.3"``

   For iOS Simulator use this command:

   ``cmake -G Xcode ../ -DCMAKE_TOOLCHAIN_FILE=../Toolchain/xcode-iphone_new.cmake -DUSE_PLUGINS=0 -DBUILD_MAPTOOL=1 -DSAMPLE_MAP=1 -DXSLTS=iphone -DUSE_UIKIT=1 -DIOS_PLATFORM=SIMULATOR``
