import serial
from PIL import Image
import numpy as np

ser = serial.Serial('/dev/tty.usbserial-A5069RR4', 115200, timeout=60)
print("Waiting")

marker = bytes([0xFF, 0xAA, 0xFF, 0xAA])
buf = b''
while True:
    buf += ser.read(1)
    if buf[-4:] == marker:
        break
    
data = ser.read(4096)
print(f"Got: {len(data)} bytes")

if len(data) == 4096:
    img = Image.fromarray(np.frombuffer(data, dtype=np.uint8).reshape((64, 64)), mode='L')
    img.save("output.png")
    img.show()