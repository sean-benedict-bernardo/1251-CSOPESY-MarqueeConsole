import os
from PIL import Image, ImageFilter

# Folder containing extracted frames
input_folder = "frames"
output_folder = "ascii_frames"
os.makedirs(output_folder, exist_ok=True)

# ASCII characters from dark to light
ASCII_CHARS = "@%#*+=-:. "

# Resize factor to avoid huge ASCII
WIDTH_SCALE = 0.5  # because characters are taller than wide
NEW_WIDTH = 25  # adjust for desired output size


def image_to_ascii(folder, file_name, new_width=NEW_WIDTH):
    image_path = f"{folder}/{file_name}"
    img = Image.open(image_path).convert("L")  # grayscale

    # filter to only edges
    img = img.filter(ImageFilter.FIND_EDGES)
    # invert image
    img = Image.eval(img, lambda x: 255 - x)

    # trim by one pixel border
    img = img.crop((1, 1, img.width - 1, img.height - 1))

    # save output image
    img.save(f"{folder}/edges_{file_name}")

    # Resize while maintaining aspect ratio
    width, height = img.size
    new_height = int((height * new_width * WIDTH_SCALE) / width)
    img = img.resize((new_width, new_height))

    # Convert pixels to ASCII chars
    pixels = img.getdata()
    ascii_str = "".join(
        ASCII_CHARS[min(pixel // 25, len(ASCII_CHARS) - 1)] for pixel in pixels
    )

    # Split into lines
    ascii_lines = [
        ascii_str[i : i + new_width] for i in range(0, len(ascii_str), new_width)
    ]
    return "\n".join(ascii_lines)


# Process all frames
for frame_file in sorted(os.listdir(input_folder)):
    # skip if file starts with edges_ to avoid reprocessing
    if frame_file.startswith("edges_"):
        continue
    if frame_file.lower().endswith((".jpg", ".png")):
        frame_path = os.path.join(input_folder, frame_file)
        ascii_art = image_to_ascii(input_folder, frame_file)

        # Save to text file
        output_path = os.path.join(
            output_folder, f"{os.path.splitext(frame_file)[0]}.txt"
        )
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(ascii_art)

print(f"Converted frames to ASCII in '{output_folder}'")
