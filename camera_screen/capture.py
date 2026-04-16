import serial
import struct
import numpy as np
import cv2

PORT = "COM4"
BAUD = 921600

print(f"Opening {PORT} at {BAUD} baud...")
ser = serial.Serial(PORT, BAUD, timeout=10)
print("Streaming... press Q to quit")

while True:
    line = ser.readline().decode("ascii", errors="ignore").strip()
    if not line:
        continue
    if line in ("ERR:TIMEOUT", "ERR:BADLEN"):
        print(f"ERROR: {line}")
        continue
    if line != "IMG:START":
        continue

    raw_len = ser.read(4)
    if len(raw_len) < 4:
        continue
    img_len = struct.unpack("<I", raw_len)[0]

    img_data = b""
    while len(img_data) < img_len:
        chunk = ser.read(img_len - len(img_data))
        if not chunk:
            break
        img_data += chunk

    ser.readline()

    # Decode and display
    jpg_array = np.frombuffer(img_data, dtype=np.uint8)
    frame = cv2.imdecode(jpg_array, cv2.IMREAD_COLOR)
    if frame is not None:
        cv2.imshow("ArduCAM Live", frame)
        print(f"Frame decoded: {img_len} bytes")
    else:
        # Save raw bytes for debugging if decode fails
        print(f"WARNING: Could not decode frame, first bytes: {img_data[:4].hex()}")
        with open("failed_frame.jpg", "wb") as f:
            f.write(img_data)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

ser.close()
cv2.destroyAllWindows()