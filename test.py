import serial
import numpy as np
ser = serial.Serial('COM3', 9600, timeout = 1)  # open serial port
TOF1_arr = []
TOF2_arr = []
numPeople = 0
TOF1_flag = 0
TOF2_flag = 0
print(ser.name)         # check which port was really used
while True:
    try:
        #print('start:')
        data = ser.readline().decode('utf-8').rstrip()
        datalist = data.split(",")
        TOF1 = float(datalist[0])
        TOF2 = float(datalist[1])
        if (TOF1 == 8191):
            TOF1_arr.append(-1)
        else:
            TOF1_arr.append(TOF1)
        if (TOF2 == 8191):
            TOF2_arr.append(-1)
        else:
            TOF2_arr.append(TOF2)
        

        TOF1_flag = (max(TOF1_arr) != -1)
        #print(TOF1_flag)
        if (TOF1_flag):
            TOF2_flag = (max(TOF2_arr) != -1)
        
        if (TOF1_flag and TOF2_flag):
            numPeople += 1
            TOF1_flag = TOF2_flag = 0
            TOF1_arr.clear()
            TOF2_arr.clear()

        #thermal = [float(x) for x in datalist[2:]]
        print("TOF1: ", TOF1)
        print("TOF2: ", TOF2)
        print("numPeople: ", numPeople)
        #matrix = np.array(thermal).reshape(8,8)
        #print(matrix)
    except UnicodeDecodeError: # catch error and ignore it
        print('uh oh')

#ser.write(b'hello')     # write a string
ser.close()             # close port