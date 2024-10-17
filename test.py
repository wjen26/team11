import serial
ser = serial.Serial('COM3', 9600, timeout = 1)  # open serial port
print(ser.name)         # check which port was really used
while True:
    try:
        print(ser.readline().decode('utf-8').rstrip())
    except UnicodeDecodeError: # catch error and ignore it
        print('uh oh')

#ser.write(b'hello')     # write a string
ser.close()             # close port