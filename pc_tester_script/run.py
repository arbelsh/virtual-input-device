import socket
import time

server = ("raspberrypi", 5000)
bufferSize = 1460
udpclient  = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send(msg,wait):
    msgBytes = str.encode(msg)
    udpclient.sendto(msgBytes, server)
    time.sleep(wait)

#Ctrl+Alt+t (open terminal) dosbox -c DAVE.EXE (run the game in dosbox)
send("CAtCA\rBBBBB  dosbox -c SdaveS.SexeS\r", 5)
# Enter (start the game)
send("\r",3)
# movement commands for the game (U-up R-right L-left)
send("RRRRRUURRRRRRRRRRRRRRUURRRRRUURRRRRRRRRRRRRRRRRRRRRRRRRLLLLLLLLLLLLLLL",1)

