import serial
import struct
import sys
import time
import matplotlib.pyplot as plt
import numpy as np


def main():
    start_time = time.time()
    try:
        ser = serial.Serial(
            port='COM5',
            baudrate=1000000,
            parity=serial.PARITY_NONE,
            timeout=None)
    except:
        sys.stdout.write("ERROR: Serial connection could not be made.")
        return

    InLine = ""
    i = 0

    while (InLine != "Subscribed.\r\n"):
        try:
            InLine = DecodeLine(ser)
            sys.stdout.write(InLine)
        except:
            return

    # while(InLine != "Peripheral disconnected\r\n"):
    while (i < 1000):
        try:
            SerialLine = ser.readline()
            # InLine = DecodeLine(ser)
            # sys.stdout.write(InLine)
            i += 1
        except:
            print("End")
            return
    timetaken = time.time()-start_time
    print(f"{i*16} bytes read.")
    print(f"Took {timetaken} seconds.")
    print(f"Throughput: {(i*16*8)/(1000*timetaken)} kbits/s.")
    ser.close()


def DecodeLine(connection):

    SerialLine = connection.readline()
    # print(sys.getsizeof(SerialLine))
    line = SerialLine.decode("utf-8")
    return line


if __name__ == "__main__":
    main()
