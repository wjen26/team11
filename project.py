# this shit does not work, do not use

import serial
import matplotlib.pyplot as plt
import numpy as np

# Setup serial communication (adjust 'COM3' and baud rate to your system)
ser = serial.Serial('/dev/tty.usbserial-0001', 9600)

# Setup plot
plt.ion()  # Turn on interactive mode
fig, ax = plt.subplots()
cax = ax.imshow(np.zeros((8, 8)), vmin=20, vmax=40, cmap='inferno')
fig.colorbar(cax)

def update_heatmap(data):
    data_array = np.array(data).reshape((8, 8))
    cax.set_data(data_array)
    plt.draw()
    plt.pause(0.01)  # Pause to allow for plot update

try:
    while True:
        line = ser.readline().decode('utf-8').strip()

        if line == "START":
            pixels = []
        elif line == "END":
            update_heatmap(pixels)
        else:
            # Split the string into float values and store in the list
            pixels = [float(value) for value in line.split(",")]

except KeyboardInterrupt:
    print("Program interrupted")

finally:
    ser.close()
