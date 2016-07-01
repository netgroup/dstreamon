#!/bin/bash

# three hosts will generate traffic versus a given server

sudo nping --tcp -S 123.94.12.1 -p 80 --flags syn www.hwupgrade.it -c 10 --delay 100ms
sudo nping --tcp -S 123.94.12.2 -p 80 --flags syn www.hwupgrade.it -c 6  --delay 100ms
sudo nping --tcp -S 123.94.12.3 -p 80 --flags syn www.hwupgrade.it -c 8  --delay 100ms
