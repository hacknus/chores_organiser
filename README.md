# chores organiser
This repository contains the C++ code in `src/main.cpp` running on an ESP8266 controlling a set of LED matrix displays (one line for each roommate).  
The ESP8266 is connected via a websocket to a python script `python/server.py` running on a Synology NAS, which does all the managing of chores for the two roommates.
