# Generated using chat GPT

import random
import argparse

def generate_matrix_file(dimension, span, output_file):
    with open(output_file, 'w') as f:
        # Write the dimension on the first line
        f.write(f"{dimension}\n")
        
        # Generate and write the matrix
        for _ in range(dimension):
            row = [str(random.randint(0, span - 1)) for _ in range(dimension)]
            f.write(' '.join(row) + '\n')


parser = argparse.ArgumentParser(description="Generate a square matrix file.")
parser.add_argument("dimension", type=int, help="Dimension of the square matrix.")
parser.add_argument("span", type=int, help="Upper bound for random numbers (exclusive).")
parser.add_argument("output", type=str, help="Output filename.")

args = parser.parse_args()
generate_matrix_file(args.dimension, args.span, args.output)