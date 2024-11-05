import serial
import numpy as np
from threading import Timer

# python3 -m serial.tools.list_ports on mac to check for port
ser = serial.Serial('/dev/cu.usbserial-0001', 9600, timeout = 1)  # open serial port
TOF1_arr = []
TOF2_arr = []
numPeople = 0
TOF1_flag = 0
TOF2_flag = 0
print(ser.name)         # check which port was really used

def clear():
    TOF1_arr.clear()
    TOF2_arr.clear()
    global timer1
    global timer2
    if timer1:
        print("canceled TOF1 flag timer")
        t1.cancel()
    if timer2:
        print("canceled TOF2 flag timer")
        t2.cancel()
    
    timer1 = 0
    
    timer2 = 0

def clearwithflag():
    TOF1_arr.clear()
    TOF2_arr.clear()
    global TOF1_flag
    TOF1_flag = 0
    global TOF2_flag
    TOF2_flag = 0
    global timer1
    timer1 = 0
    global timer2
    timer2 = 0
    print("cleared TOF flags")

def Buffer_clear():
    TOF1_arr.clear()
    TOF2_arr.clear()
    t = Timer(1, clear)
    t.start()


timer1 = 0
timer2 = 0

while True:
    try:
        #print('start:')
        data = ser.readline().decode('utf-8').rstrip()
        datalist = data.split(",")
        TOF1 = float(datalist[0])
        TOF2 = float(datalist[1])
        if (TOF1 == 8191 or TOF1 == -1):
            TOF1_arr.append(np.nan)
        else:
            TOF1_arr.append(TOF1)
        if (TOF2 == 8191 or TOF2 == -1):
            TOF2_arr.append(np.nan)
        else:
            TOF2_arr.append(TOF2)
        

        TOF1_flag = not np.all(np.isnan(TOF1_arr))
        TOF2_flag = not np.all(np.isnan(TOF2_arr))
        #print(TOF1_flag)
        
        
        if (TOF1_flag and np.isnan(TOF1_arr[-1]) and not timer1):
            print("started TOF1 flag clear timer")
            t1 = Timer(5, clearwithflag)
            t1.start()
            timer1 = 1
        if (TOF2_flag and np.isnan(TOF2_arr[-1]) and not timer2):
            print("started TOF2 flag clear timer")
            t2 = Timer(5, clearwithflag)
            t2.start()
            timer2 = 1


        if (TOF1_flag and TOF2_flag):
            TOF1_index = TOF1_arr.index(np.nanmin(TOF1_arr))
            TOF2_index = TOF2_arr.index(np.nanmin(TOF2_arr))
            if TOF1_index < TOF2_index:
                print("person entering")
                numPeople += 1

                
            elif TOF2_index <= TOF1_index:
                print("person exiting")
                print(TOF1_index)
                print(TOF2_index)
                print(TOF2_arr)
                numPeople -= 1
                

            else:
                print("WTF DAWG")
            TOF1_flag = 0
            TOF2_flag = 0
            Buffer_clear()


            
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