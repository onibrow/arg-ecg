#!/bin/bash

read -p 'Enter address: ' addr
echo 
echo Connecting to $addr
echo

while [ 1 ]
do
  curl http://$addr/L
  sleep 1
  curl http://$addr/H
  sleep 1 
done
