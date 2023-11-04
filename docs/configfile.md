# The Config File

With ProffieConfig, you shouldn't need to deal with ProffieOS config files for the large majority of operation, but there may be some cases in which doing so is necessary.

ProffieConfig allows you to import, export, and verify configuration files.

## Import

Although a dialog is presented on first run, it may be desired to import a new/different config file instead. To do this, go to File->Import Config... and select the file to import. Provided the config file uses supported features, it will be sucessfully imported.

***NOTE: Importing a config file with PERMENANTLY ERASE the current open configuration and setings in ProffieConfig. If you want to save your current configuration file, be sure to export it before importing a new one!***

## Export

File->Export Config... will export the generated ProffieOS config file created by ProffieConfig, which can be used if you're ready to delve into advanced customization, or if it's requested for debugging.

This file can be exported anywhere, doing so has no effect on the configuration in ProffieConfig, which will remain loaded/saved.

## Verify

Sometimes it may be desirable to test and ensure there is nothing conflicting or erroneous in a config while working on it, but you may not want to have to plug in the board and try applying the configuration just to test. In this case, running "Verify Config..." will run the same steps as there would be when applying changes, but it can be done without a board conneted, and if it is, it will not apply the changes yet.
