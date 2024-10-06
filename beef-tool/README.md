# beef-board firmware upgrade utility
1. Run `drivers/install_all_drivers.bat`
2. Place your fw file in the same directory as `beef-tool.exe`
3. Make sure your fw file is named `beef.hex`
4. Run `beef-tool.exe`
5. Put beef-board in programming mode
6. Click the button in the UI to upgrade the FW
7. Unplug and replug your device

`beef-tool.exe` may trigger windows antivirus. This is a _false
positive_ and may be safely ignored.