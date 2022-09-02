import serial
import struct
import sys
import time
import matplotlib.pyplot as plt
import numpy as np


def main():
    try:
        ser = serial.Serial(
            port='COM5',
            baudrate=1000000,
            parity=serial.PARITY_NONE,
            timeout=5)
    except:
        sys.stdout.write("ERROR: Serial connection could not be made.")
        return

    #mode = ""
    #while (mode != "Read" and mode != "Write"):
    #    mode = input("Enter \"Read\" for a read operation and \"Write\" for a write operation: ")

    xdata = np.array([])
    ydata = np.array([])
    zdata = np.array([])
    readTime = np.array([])
    InLine = ""
    i = 0

    while (InLine != "Subscribed.\r\n"):
        try:
            InLine = DecodeLine(ser)
            sys.stdout.write(InLine)
        except:
            return

    plt.close('all')
    plt.figure()
    plt.ion()
    plt.show()

    start_time = time.time()
    i = 0
    while (i<1000):
        if ser.in_waiting:
            try:
                InLine = DecodeLine(ser)
                if (InLine == "Peripheral disconnected\r\n"):
                    plt.close()
                    break
                InData = InLine.split(' ', 3)
                InData[3] = InData[3][:-2]
                # print(InData)
                InData[0] = struct.unpack('<f', bytes.fromhex(InData[0]))[0]
                InData[1] = struct.unpack('<f', bytes.fromhex(InData[1]))[0]
                InData[2] = struct.unpack('<f', bytes.fromhex(InData[2]))[0]
                InData[3] = struct.unpack('<L', bytes.fromhex(InData[3]))[0]
                xdata = np.append(xdata, InData[0])
                # print(xdata)
                ydata = np.append(ydata, InData[1])
                zdata = np.append(zdata, InData[2])
                readTime = np.append(readTime, InData[3])
                xdata = xdata[-50:]
                ydata = ydata[-50:]
                zdata = zdata[-50:]
                readTime = readTime[-50:]
                plt.cla()
                plt.plot(readTime, xdata, label="X direction")
                plt.plot(readTime, ydata, label="Y direction")
                plt.plot(readTime, zdata, label="Z direction")
                plt.subplots_adjust(bottom=0.30)
                plt.title('Plot of Serial Port Data')
                plt.ylabel('Acceleration')
                plt.xlabel('Time')
                plt.legend()
                plt.pause(0.04)
            except:
                print(InLine)
                ser.reset_input_buffer()
                time.sleep(0.2)
            i += 1
    plt.close('all')
    ser.close()
    timetaken = time.time()-start_time
    print(f"{i*16} bytes read.")
    print(f"Took {timetaken} seconds.")
    print(f"Throughput: {(i*16*8)/(1000*timetaken)} kbits/s.")


def DecodeLine(connection):

    SerialLine = connection.readline()
    # print(SerialLine)
    line = SerialLine.decode("utf-8")
    return line


if __name__ == "__main__":
    main()
