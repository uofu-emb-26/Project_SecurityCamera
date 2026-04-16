from PIL import Image
import numpy as np

img = Image.open("coin.jpeg").convert("L")
img = img.resize((64, 64))
data = np.array(img, dtype=np.uint8).flatten()

with open("image_data.h", "w") as f:
    f.write("#define IMAGE_SIZE 4096\n")
    f.write("uint8_t image[IMAGE_SIZE] = {\n")
    f.write(", ".join(str(x) for x in data))
    f.write("\n};\n")