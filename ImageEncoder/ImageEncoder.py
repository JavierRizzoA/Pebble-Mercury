#!/usr/bin/python3

import argparse
from bitarray import bitarray
from PIL import Image

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

def read_arguments():
    parser = argparse.ArgumentParser(
            prog='ImageEncoder',
            description='Converts a monochromatic image to a binary-encoded 1-bit image'
            );
    parser.add_argument('-i', '--input', required=True, type=str)
    parser.add_argument('-o', '--output', required=True, type=str)
    parser.add_argument('--invert', required=False, default=False, action='store_true')
    return parser.parse_args()

if __name__ == '__main__':
    args = read_arguments()

    image = Image.open(args.input)
    image = image.convert('RGB')
    pixels = image.load()
    width, height = image.size

    print(args.input)
    print(f'Image size: {image.size}')
    print(pixels[1, 0])

    bits = bitarray(endian='little')

    for y in range(height):
        for x in range(width):
            color = pixels[x, y]
            if color == BLACK:
                bits.append(not args.invert)
            elif color == WHITE:
                bits.append(args.invert)
            else:
                raise ValueError(f'Invalid color in image: {color}');

    while not len(bits) % 8 == 0:
        bits.append(False)

    with open(args.output, 'wb') as f:
        bits.tofile(f)
