# Building a release

The init.sh and CMake builds takes care of organizing stuff into the staging and
deploy folders, but from there, there's more work to be done.

The `upgen` utility organizes files and generates the launcher manifest needed,
and is the lovely source of the ugliest code in this repository... which is
impressive.

Once the assets are organized they're sent to the delivery server.

The launcher however needs special handling for each platform, as it's the
"native" looking downloadable point.

# Windows

Actually I lied, Windows is easy. The executable gets renamed and that's it.

# Linux

The AppDir layout for the AppImage is setup first.
`linuxdeploy --appdir=deploy/[dir]/AppDir --desktop-file=resources/templates/proffieconfig.desktop --executable=deploy/[dir]/ProffieConfig --icon-file=resources/icons/icon-thumb.png --icon-filename=proffieconfig`

Then the metainfo file has to be copied in
`mkdir deploy/[dir]/AppDir/usr/share/metainfo`
`cp resources/templates/com.kafrenetrading.proffieconfig.appdata.xml deploy/[dir]/AppDir/usr/share/metainfo`

Then the AppImage is built
`appimagetool deploy/[dir]/AppDir deploy/[dir]/ProffieConfig_Launcher-[arch]-[version].AppImage`

# macOS

The app bundle is handled by the CMake files.

The DMG file is built with the `create-dmg` utility:
`create-dmg --volname "ProffieConfig Launcher" --volicon resources/icons/icon.icns --window-size 350 300 --window-pos 400 400 --icon-size 96 --icon ProffieConfig.app 175 100 ProffieConfig_Launcher-[arch]-[version].dmg deploy/[dir]/`

