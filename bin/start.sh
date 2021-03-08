#!/usr/bin/env bash
CNCJS_SECRET=$(cat /home/pi/.cncjs/cncrc.cfg | jq -r '.secret') /home/pi/cncjs-pendant-joystick/bin/cncjs-pendant-joystick2 --joystick=/dev/ttyACM1 -p /dev/ttyACM0
