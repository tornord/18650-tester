from datetime import date

from PIL import Image, ImageDraw, ImageFont


def make_png(filename, width, height, line1, line2, font_path, font_size):
    # Create a white background
    img = Image.new("RGB", (width, height), color="white")
    draw = ImageDraw.Draw(img)

    #draw.rectangle([0, 0, width - 1, height - 1], outline="black", width=1)

    # Load font
    try:
        font = ImageFont.truetype(font_path, font_size)
    except OSError:
        font = ImageFont.load_default()
        print("font error")

    # Get text size and position line1
    bbox1 = draw.text((4, -7), line1, font=font, fill="black")
    # w1, h1 = bbox1[2] - bbox1[0], bbox1[3] - bbox1[1]
    # x1 = (width - w1) // 2
    # y1 = height // 3 - h1 // 2

    # Line2
    bbox2 = draw.text((4, 34), line2, font=font, fill="black")
    # w2, h2 = bbox2[2] - bbox2[0], bbox2[3] - bbox2[1]
    # x2 = (width - w2) // 2
    # y2 = (2 * height) // 3 - h2 // 2

    # Save to file
    img.save(filename, "PNG")


if __name__ == "__main__":
    # Example usage:

    today_str = date.today().strftime("%y%m")
    capacity = 2.3
    internal_resistance = 0.092

    make_png(
        filename="output.png",
        width=306,
        height=70,
        line1=today_str,
        line2=f"{10*capacity:.0f}dAh" + " | " + f"{100*internal_resistance:.0f}cΩ",
        font_path="Monaco.ttf",  # path to your .ttf font file
        font_size=36,
    )
