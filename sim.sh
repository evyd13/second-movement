#!/bin/bash
rm -r build-sim
emmake make BOARD=sensorwatch_pro DISPLAY=custom
python3 -m http.server -d build-sim
