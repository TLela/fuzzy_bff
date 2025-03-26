import matplotlib.pyplot as plt
import csv
import numpy as np

# Number of sizes we test for (change this if you change the size/testsize vectors in test_performance.cpp)
n = 4

# Read data from file
size = []
filtersize_fuzzy = []
filtersize = []
constructiontime_fuzzy = []
constructiontime = []
querytime_fp_fuzzy = []
querytime_fp = []
querytime_fn_fuzzy = []
querytime_fn = []

with open('../Results/compare_BFF_fBFF.txt', 'r') as file:
    reader = csv.reader(file)
    next(file)  # Skip the first line
    constr_temp = 0
    query_fp_temp = 0
    query_fn_temp = 0
    for index,row in enumerate(reader):
        if (index+1) % 3 != 0:
            constr_temp += float(row[3])/1000
            query_fp_temp += float(row[4])/1000
            query_fn_temp += float(row[5])/1000
        else:
            if index < 3*n:
                constr_temp += float(row[3])/1000
                query_fp_temp += float(row[4])/1000
                query_fn_temp += float(row[5])/1000

                size.append(int(row[0]))
                filtersize_fuzzy.append(int(row[2]))
                constructiontime_fuzzy.append(constr_temp/3)
                querytime_fp_fuzzy.append(query_fp_temp/3)
                querytime_fn_fuzzy.append(query_fn_temp/3)

                constr_temp = 0
                query_fp_temp = 0
                query_fn_temp = 0

            else:
                constr_temp += float(row[3])/1000
                query_fp_temp += float(row[4])/1000
                query_fn_temp += float(row[5])/1000
                
                filtersize.append(int(row[2]))
                constructiontime.append(constr_temp/3)
                querytime_fp.append(query_fp_temp/3)
                querytime_fn.append(query_fn_temp/3)

                constr_temp = 0
                query_fp_temp = 0
                query_fn_temp = 0



# Create the figure and axis
plt.figure(figsize=(8, 5))

# Plot with markers and linewidth
plt.plot(size, constructiontime, label='BFF', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, constructiontime_fuzzy, label='BFF Fuzzy',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Size', fontsize=12)
plt.ylabel('Time (millisec.)', fontsize=12)
plt.title('Constructiontime', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)
plt.savefig("../Plots/Constructiontime.png")
plt.show()

# Create the figure and axis
plt.figure(figsize=(8, 5))

# Plot with markers and linewidth
plt.plot(size, filtersize, label='BFF', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, filtersize_fuzzy, label='BFF Fuzzy',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Size', fontsize=12)
plt.ylabel('Filtersize', fontsize=12)
plt.title('Filtersize', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Filtersize.png")
plt.show()

# Create the figure and axis
plt.figure(figsize=(8, 5))

# Plot with markers and linewidth
plt.plot(size, querytime_fn, label='BFF in set', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, querytime_fn_fuzzy, label='BFF Fuzzy in set',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')
plt.plot(size, querytime_fp, label='BFF not in set', linestyle='-', linewidth=2, markersize=5, color='#2ca02c')
plt.plot(size, querytime_fp_fuzzy, label='BFF Fuzzy not in set',  linestyle='-', linewidth=2, markersize=5, color='#d62728')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Size', fontsize=12)
plt.ylabel('Querytime', fontsize=12)
plt.title('Querytime', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Querytime.png")
plt.show()

# Create the figure and axis
plt.figure(figsize=(8, 5))

# Plot with markers and linewidth
plt.plot(size, querytime_fn, label='BFF in set', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, querytime_fp, label='BFF not in set', linestyle='-', linewidth=2, markersize=5, color='#2ca02c')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Size', fontsize=12)
plt.ylabel('Querytime', fontsize=12)
plt.title('Querytime BFF', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Querytime_BFF.png")
plt.show()

# Create the figure and axis
plt.figure(figsize=(8, 5))

# Plot with markers and linewidth
plt.plot(size, querytime_fn_fuzzy, label='BFF Fuzzy in set',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')
plt.plot(size, querytime_fp_fuzzy, label='BFF Fuzzy not in set',  linestyle='-', linewidth=2, markersize=5, color='#d62728')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Size', fontsize=12)
plt.ylabel('Querytime', fontsize=12)
plt.title('Querytime fuzzy BFF', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Querytime_fuzzy_BFF.png")
plt.show()