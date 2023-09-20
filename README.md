# Eclipse Installation Management Tool (EC Updater)



EC Updater tool enables administrators to quickly deploy Eclipse IDE on Linux machines. It also comes with "Interactive Mode" for beginners, which they can install Eclipse IDE through few clicks.



## Usage

#### Main Operating Modes

- **-i** : Drop into Interactive Mode

- **-s** : To Install Eclipse

- **-u** : To Upgrade Eclipse

- **-r** : To Remove Eclipse

- **-c** : To Configure EC Updater

#### Parameters

- **-d /some/dirs** : Applying operation at /some/dirs (Default: EC Updater directory)

- **-v YYYY-MM/R** : Selecting Eclipse version. (Default: Latest stable release)

- **-p product** : Select Eclipse product. (Mandatory when installing Eclipse (-s))

- **-b** : Treating beta release as latest release.

- **-l** : Verbose mode.



## Notes

- Please do not delete or modify <i>config.conf</i>. Please use **EC_Updater -c** instead of editing the config file.

- It is quite easy to localize EC_Updater. You can do that by simply copying the **en_US.lang** and **en_US.hlp** as template, and start localizing it.

 
