#!/usr/bin/env bash
CNCJS_SECRET=$(cat /home/pi/.cncjs/cncrc.cfg | jq -r '.secret') /home/pi/cncjs-pendant-joystick/bin/cncjs-pendant -d /dev/ttyACM0 -p /dev/ttyACM1