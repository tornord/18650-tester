from datetime import date

from brother_ql.conversion import convert
from brother_ql.backends.helpers import send
from brother_ql.raster import BrotherQLRaster

from make_png import make_png


def create_and_print_label(capacity=2.6, internal_resistance=0.09):
    """
    Create and print a label with the given capacity and internal resistance values.

    Args:
        capacity: Battery capacity value (default: 2.6)
        internal_resistance: Internal resistance value (default: 0.09)

    Returns:
        dict: Information about the generated label
    """
    try:
        # Generate date string in "mmm yyyy" format
        today_str = date.today().strftime("%b %Y")

        # Format capacity and internal resistance strings
        capacity_str = f"{10*capacity:.0f}dAh"
        internal_resistance_str = f"{100*internal_resistance:.0f}cΩ"

        # Create label lines
        line1 = today_str
        line2 = capacity_str + " | " + internal_resistance_str
        font_size = 36

        # Generate PNG image
        make_png(
            filename="output.png",
            width=306,
            height=70,
            line1=line1,
            line2=line2,
            font_path="Monaco.ttf",  # path to your .ttf font file
            font_size=font_size,
        )

        # Configure printer
        backend = "pyusb"
        model = "QL-700"
        printer = "usb://0x04f9:0x2042"
        qlr = BrotherQLRaster(model)
        qlr.exception_on_warning = True

        # Print label
        kwargs = {
            "cut": True,
            "label": "29",
            "images": ["output.png"],
        }
        instructions = convert(qlr=qlr, **kwargs)

        send(
            instructions=instructions,
            printer_identifier=printer,
            backend_identifier=backend,
            blocking=True,
        )

        return {
            "date": today_str,
            "capacity": capacity,
            "capacity_str": capacity_str,
            "internal_resistance": internal_resistance,
            "internal_resistance_str": internal_resistance_str,
            "line1": line1,
            "line2": line2,
            "image_file": "output.png",
        }

    except Exception as e:
        raise Exception(f"Failed to create and print label: {str(e)}")


# Main execution (for direct script usage)
if __name__ == "__main__":
    # Default values for direct script execution
    # capacity = 1.4
    # internal_resistance = 0.2

    # print(
    #     f"Creating label with capacity: {capacity}, internal resistance: {internal_resistance}"
    # )

    # result = create_and_print_label(capacity, internal_resistance)
    # print("Label created successfully!")
    # print(f"Result: {result}")


    make_png(
        filename="output.png",
        width=306,
        height=70,
        line1="ETHERNET",
        line2="RJ45 8-PIN",
        font_path="Monaco.ttf",  # path to your .ttf font file
        font_size=36,
    )

    backend = "pyusb"
    model = "QL-700"
    printer = "usb://0x04f9:0x2042"
    qlr = BrotherQLRaster(model)
    qlr.exception_on_warning = True

    kwargs = {
        "cut": True,
        "label": "29",
        "images": ["output.png"],
    }
    instructions = convert(qlr=qlr, **kwargs)

    send(
        instructions=instructions,
        printer_identifier=printer,
        backend_identifier=backend,
        blocking=True,
    )