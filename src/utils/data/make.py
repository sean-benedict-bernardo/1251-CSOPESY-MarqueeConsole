import pyfiglet

DIRECTORY = "characters/"

texts = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890")
# texts = list("ABC")


def make_ascii_art(text: str) -> str:
    """
    Convert the given text to ASCII art using pyfiglet.

    Args:
        text (str): The text to convert.

    Returns:
        str: The ASCII art representation of the text.
    """
    ascii_art = pyfiglet.figlet_format(text, font="big")

    ascii_art = ascii_art.split("\n")

    # remove empty rows
    ascii_art = [row for row in ascii_art if row.strip() != ""]

    # for i in range(len(ascii_art)):
    #     ascii_art[i] = ascii_art[i].strip()

    return "\n".join(ascii_art)


def export_ascii_art_to_file(text: str, filename: str) -> None:
    """
    Export the utf-8 art of the given text to a file.

    Args:
        text (str): The text to convert.
        filename (str): The name of the file to save the ASCII art.
    """
    ascii_art = make_ascii_art(text)
    with open(filename, "w", encoding="utf-8") as f:
        f.write(ascii_art)


if __name__ == "__main__":
    for letter in texts:
        export_ascii_art_to_file(letter, f"{DIRECTORY}{letter}.txt")
