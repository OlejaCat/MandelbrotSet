import numpy as np
import argparse

from matplotlib import pyplot as plt


def analyze_mandelbrot(filenames: list[str]):
    for file in filenames:
        title = ''
        data = []
        with open(file, 'r') as input_file:
            title = input_file.readline() 
            data = np.loadtxt(input_file)

        mean = data.mean()
        std = data.std(ddof=1)
        error = std / np.sqrt(len(data))

        filtered_data = data[(data >= mean - 2 * std) & (data <= mean + 2 * std)]

        print(f"File: {file}")
        print(f"Mean: {mean:.0f} ± {error:.0f} ticks")
        print(f"Percent of filtered data:", end=' ') 
        print(f"{len(filtered_data) / len(data) * 100:.2f}%")


        plt.figure(figsize=(10, 6))
        plt.hist(filtered_data, bins=50, zorder=2)

        plt.axvline(
            mean, 
            color='r', 
            linestyle='--', 
            label=f"Среднее: {mean/1e8:.3f} $\\times$ 10$^{{8}}$",
            zorder=3
        )
        plt.axvline(mean + 2 * std, color='g', linestyle=':', label='±2σ')
        plt.axvline(mean - 2 * std, color='g', linestyle=':')

        plt.ylabel("Плотность")
        plt.xlabel("Такты процессора")
        plt.title(title)
        plt.grid(True, zorder=0, alpha=0.95)
        plt.legend()
        plt.savefig(f"{file.rstrip(".txt")}.png", dpi=250)
        plt.close() 


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Plots for mandelbrot benchmark"
    )

    parser.add_argument('-f', '--file', nargs='+', required=True)
    args = parser.parse_args()
    analyze_mandelbrot(args.file)
