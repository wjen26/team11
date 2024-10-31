import serial
import numpy as np
ser = serial.Serial('COM3', 9600, timeout = 1)  # open serial port
print(ser.name)         # check which port was really used
while True:
    try:
        #print('start:')
        data = ser.readline().decode('utf-8').rstrip()
        datalist = data.split(",")
        TOF1 = float(datalist[0])
        thermal = [float(x) for x in datalist[1:]]
        print(TOF1)
        matrix = np.array(thermal).reshape(8,8)
        print(matrix)
    except UnicodeDecodeError: # catch error and ignore it
        print('uh oh')

#ser.write(b'hello')     # write a string
ser.close()             # close port