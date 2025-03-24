import matplotlib.pyplot as plt
import csv
import numpy as np

# Read BFF data from file
i = []
bff_sing = []
with open('BFFTEST.txt', 'r') as file:
    reader = csv.reader(file)
    next(file)  # Skip the first line
    for row in reader:
        i.append(float(row[0]))
        bff_sing.append(float(row[1]))

# Read BFFwrapping data from file
bffwrap_sing = []
with open('BFFWRAPTEST.txt', 'r') as file:
    reader = csv.reader(file)
    next(file)  # Skip the first line
    for row in reader:
        bffwrap_sing.append(float(row[1]))


# Create the figure and axis
plt.figure(figsize=(8, 5))

# Plot with markers and linewidth
plt.plot(i, bff_sing, label='BFF', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(i, bffwrap_sing, label='BFF Wrapping',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')

# Log scale for y-axis
#plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Segment index', fontsize=12)
plt.ylabel('Number of Singletons', fontsize=12)
plt.title('Singleton Distribution Comparison', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.show()
