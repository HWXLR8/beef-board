# Beef Board SpiceTools companion scripts

## iidx_light_reader.py

This Python script reads tape LED data from Spice and outputs it to your Phoenixwan's centre bar lights.

You will need to put your light bar's LED mode to `Tape LED (P1/P2)` either in the web config or by cycling through the modes with the hot key `B6 + B8 + B10`. You can see if you have selected the correct mode if there is a white light scrolling from top to bottom.

You will also need to setup SpiceAPI by either adding `-api <port number e.g. 8080>` to your launch arguments in your game startup script, or by setting it in `spicecfg.exe` under the API tab. The setting should be called something like `API TCP Port`.

To use this script, you first need to install the script's dependencies.

```bash
pip install -r requirements.txt
```

Next, open up the `iidx_light_reader.py` script and go to the line near the top where it says `sys.path.append("/path/to/spice2x/api/resources/python")`. You will need to change the string to your local copy of the SpiceAPI Python library. You can find this in any spice2x release. If you already have the SpiceAPI library in your Python path, you can skip this step.

Then pass in the API port number you chose.

```bash
python iidx_light_reader.py --port <port number>
```

If you have a password set for SpiceAPI you can also pass that in with the `--password` argument.

Now start the game. Your controller should set the centre bar lights off when the game is launched, which indicates the script is working correctly.

## Troubleshooting

Make sure you have the latest Beef Board firmware flashed onto your board as well as a recent version of spice2x.

If, for whatever reason, the script isn't working properly, check if there are any error or warning messages from the script. If you can't figure out the reason for the message, then create a new Github issue in the beef-board repository with the details of your message and setup.
