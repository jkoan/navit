# Google Glass Headup Plugin

The Google Glass Headup Plugin \<https://www.eisenzelt.de/\> enables Navit to show navigation data on a Google Glass Xplorer Edition device with latest firmware. It is supported by Linux and iOS Navit devices currently.

## Quick Start

- Build Navit from source and supply the cmake argument ```-Dplugin/googleglass=true``` on the commandline.

- Clone https://github.com/OLFDB/navit-glassheadup.git.

- Find the line ```fos.write(new String("deviceserial=35895448783026136412").getBytes());```in ```ConnectionManager.java```and enter a random serial number.

  ```
  fos.write(new String("deviceserial=35895448783026136412").getBytes());
  ```

- Build it using Android Studio 4.2.2 and install it to your Glass device.

- Edit your navit.xml to contain a line like:

``` xml
<headup type="googleglass" deviceserial="35895448783026136412"/>
```

- Start navit.

- Start the "Show Speed" app on the Glass device.

- It should connect and show some navigation data like:

  ![image-20250216152228242](/Volumes/UserData/Library/Application Support/typora-user-images/image-20250216152228242.png)

Be aware that the Bluetooth LE Stack of Android 4.4.1 is a mess. Connectivity is working best in a "silent" Bluetooth environment with a low amount of advertising devices. Pick up your phone and hold it next to the Glass device when connecting. The lowest line on the screenshot shows data from the OBD2 plugin and will only be shown, when available. 
