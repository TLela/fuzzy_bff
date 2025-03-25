import matplotlib.pyplot as plt
import csv
import numpy as np

runs = 10

# Create the figure and axis
plt.figure(figsize=(8, 5))

# Read BFF data from file
i = []
bff_sing_rel = []
with open("../Results/BFFTEST_0_0.txt", 'r') as file:
    reader = csv.reader(file)
    next(file)  # Skip the first line
    for row in reader:
        i.append(float(row[0]))
        bff_sing_rel.append(float(row[3]))

for countRuns in range(1,runs):
    filename = '../Results/BFFTEST_0_' + str(countRuns) + '.txt'
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        next(file)  # Skip the first line
        for index, row in enumerate(reader):
            bff_sing_rel[index] += float(row[3])

for index, value in enumerate(bff_sing_rel):
    bff_sing_rel[index] = value / runs

# Plot with markers and linewidth
plt.plot(i, bff_sing_rel, label='0%', linestyle='-', linewidth=1.5, markersize=2)

bff_sing_rel = []
filename = "../Results/BFFTEST_" + str(4) + "_0.txt"
with open(filename, 'r') as file:
    reader = csv.reader(file)
    next(file)  # Skip the first line
    for row in reader:
        bff_sing_rel.append(float(row[3]))

for countRuns in range(1,runs):
    filename = '../Results/BFFTEST_' + str(4) + '_' + str(countRuns) + '.txt'
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        next(file)  # Skip the first line
        for index, row in enumerate(reader):
            bff_sing_rel[index] += float(row[3])

for index, value in enumerate(bff_sing_rel):
    bff_sing_rel[index] = value / runs

# Plot with markers and linewidth
plt.plot(i, bff_sing_rel, label= str((4)*25) + '%', linestyle='-', linewidth=1.5, markersize=2)

for j in range(1,4):

    bff_sing_rel = []
    filename = "../Results/BFFTEST_" + str(j) + "_0.txt"
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        next(file)  # Skip the first line
        for row in reader:
            bff_sing_rel.append(float(row[3]))

    for countRuns in range(1,runs):
        filename = '../Results/BFFTEST_' + str(j) + '_' + str(countRuns) + '.txt'
        with open(filename, 'r') as file:
            reader = csv.reader(file)
            next(file)  # Skip the first line
            for index, row in enumerate(reader):
                bff_sing_rel[index] += float(row[3])

    for index, value in enumerate(bff_sing_rel):
        bff_sing_rel[index] = value / runs

    # Plot with markers and linewidth
    plt.plot(i, bff_sing_rel, label= str((j)*25) + '%', linestyle='--', linewidth=1.5, markersize=2)



# Log scale for y-axis
#plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Segment index', fontsize=12)
plt.ylabel(' Singleton/Total Elements', fontsize=12)
plt.title('Singleton Distribution During BFF Construction (Relative)', fontsize=14, fontweight='bold')

# Add a legend with better positioning
custom_order = [str((j)*25) + '%' for j in range(5)]  # Define desired order
handles, labels = plt.gca().get_legend_handles_labels()
label_to_handle = dict(zip(labels, handles))  # Map labels to handles
plt.legend([label_to_handle[label] for label in custom_order], custom_order,loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig('../Plots/singletonplot_overtime_relative.png')
plt.show()
