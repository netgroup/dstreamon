#!/bin/bash

# bad guy will send 13 ping SYN request to server (above the average of 8)

sudo nping --tcp -S 123.94.12.4 -p 80 --flags syn www.hwupgrade.it -c 13 --delay 30ms
