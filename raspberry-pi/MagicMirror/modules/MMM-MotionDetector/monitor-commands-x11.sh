#!/bin/bash

export DISPLAY=:0
export XAUTHORITY=/home/fsmt2-frontend/.Xauthority

case "$1" in
  on)
    xset dpms force on
    ;;
  off)
    xset dpms force off
    ;;
  status)
    STATE=$(xset -q | awk '/Monitor is/ {print $3}')
    if [ "$STATE" = "Off" ] || [ "$STATE" = "Standby" ] || [ "$STATE" = "Suspend" ]; then
      echo "OFF"
    else
      echo "ON"
    fi
    ;;
  *)
    echo "Usage: $0 {on|off|status}"
    exit 1
    ;;
esac
