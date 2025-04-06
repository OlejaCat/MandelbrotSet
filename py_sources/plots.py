import numpy as np
import argparse

from matplotlib import pyplot as plt


def analyze_mandelbrot(filenames: list[str]):
    fig, axes = plt.subplots(1, 2, figsize=(20, 8))
    plt.subplots_adjust(wspace=0.3)

    for idx, file in enumerate(filenames):
        ax = axes[idx]

        title = ''
        data = []
        with open(file, 'r') as input_file:
            title = input_file.readline() 
            data = np.loadtxt(input_file)

        mean = data.mean()
        std = data.std(ddof=1)
        error = std / np.sqrt(len(data))

        filtered_data = data[(data >= mean - std) & (data <= mean + std)]

        print(f"File: {file}")
        print(f"Mean: {mean:.0f} ± {error:.0f} ticks")
        print(f"Percent of filtered data:", end=' ') 
        print(f"{len(filtered_data) / len(data) * 100:.2f}%")

        ax.hist(filtered_data, bins=30, zorder=2)

        ax.axvline(
            mean, 
            color='r', 
            linestyle='--', 
            label=f"Среднее: {mean/1e8:.3f} $\\times$ 10$^{{8}}$",
            zorder=3
        )
        ax.axvline(mean + std, color='g', linestyle=':', label='±σ')
        ax.axvline(mean - std, color='g', linestyle=':')

        ax.set_ylabel("Плотность")
        ax.set_xlabel("Такты процессора")
        ax.set_title(title)
        ax.set_yscale('log')
        ax.grid(True, zorder=0, alpha=0.95)
        ax.legend()

    output_name = f"comprasion_{'_vs_'.join(x.rstrip(".txt").lstrip("results/") for x in filenames)}"
    plt.tight_layout()
    plt.savefig(f"{output_name}.png", dpi=300)
    plt.close() 
    


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Plots for mandelbrot benchmark"
    )

    parser.add_argument('-f', '--file', nargs='+', required=True)
    args = parser.parse_args()
    analyze_mandelbrot(args.file)
