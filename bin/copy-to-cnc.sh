#!/usr/bin/env bash
rsync -avzh --exclude ".git" pi@192.168.0.216:/home/pi/cncjs-pendant-joystick ~/projects/ 
rsync -avz --exclude ".git" ~/projects/cncjs-pendant-joystick pi@cnc.lan:~ 
