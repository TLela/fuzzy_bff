import matplotlib.pyplot as plt
import csv
import numpy as np

# Adjust accordingly if you change the number of runs in test_performance.cpp
# Number of sizes we test for (change this if you change the size/testsize vectors in test_performance.cpp)
n = 6
reps = 5

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
fp_fuzzy = []
fn_fuzzy = []
fn = []
fp = []
fp_expected_fuzzy = []
fn_expected_fuzzy = []
fp_expected = []
fn_expected = []

with open('../Results/compare_BFF_fBFF.txt', 'r') as file:
    reader = csv.reader(file)
    next(file)  # Skip the first line
    constr_temp_fuzzy = 0
    query_fp_temp_fuzzy = 0
    query_fn_temp_fuzzy = 0
    fp_temp_fuzzy = 0
    fn_temp_fuzzy = 0
    constr_temp = 0
    query_fp_temp = 0
    query_fn_temp = 0
    fp_temp = 0
    fn_temp = 0
    for index,row in enumerate(reader):
        if index % 2 == 0:
            if index % (2*reps) != 2*reps-2:
                constr_temp_fuzzy += float(row[3])/1000
                query_fp_temp_fuzzy += float(row[4])/1000
                query_fn_temp_fuzzy += float(row[5])/1000
                fp_temp_fuzzy += float(row[6])
                fn_temp_fuzzy += float(row[7])
            else:
                constr_temp_fuzzy += float(row[3])/1000
                query_fp_temp_fuzzy += float(row[4])/1000
                query_fn_temp_fuzzy += float(row[5])/1000
                fp_temp_fuzzy += float(row[6])
                fn_temp_fuzzy += float(row[7])

                size.append(int(row[0]))
                filtersize_fuzzy.append(int(row[2]))
                fp_expected_fuzzy.append(float(row[8]))
                fn_expected_fuzzy.append(float(row[9]))
                constructiontime_fuzzy.append(constr_temp_fuzzy/reps)
                querytime_fp_fuzzy.append(query_fp_temp_fuzzy/reps)
                querytime_fn_fuzzy.append(query_fn_temp_fuzzy/reps)
                fp_fuzzy.append(fp_temp_fuzzy/reps)
                fn_fuzzy.append(fn_temp_fuzzy/reps)

                constr_temp_fuzzy = 0
                query_fp_temp_fuzzy = 0
                query_fn_temp_fuzzy = 0
                fp_temp_fuzzy = 0
                fn_temp_fuzzy = 0

        else:
            if index % (2*reps) != 2*reps-1:
                constr_temp += float(row[3])/1000
                query_fp_temp += float(row[4])/1000
                query_fn_temp += float(row[5])/1000
                fp_temp += float(row[6])
                fn_temp += float(row[7])
            else:
                constr_temp += float(row[3])/1000
                query_fp_temp += float(row[4])/1000
                query_fn_temp += float(row[5])/1000
                fp_temp += float(row[6])
                fn_temp += float(row[7])

                filtersize.append(int(row[2]))
                fp_expected.append(float(row[8]))
                fn_expected.append(float(row[9]))
                constructiontime.append(constr_temp/reps)
                querytime_fp.append(query_fp_temp/reps)
                querytime_fn.append(query_fn_temp/reps)
                fp.append(fp_temp/reps)
                fn.append(fn_temp/reps)

                constr_temp = 0
                query_fp_temp = 0
                query_fn_temp = 0
                fp_temp = 0
                fn_temp = 0


# print factor of filtersize
sum = 0
for i in range(len(filtersize_fuzzy)):
    sum += filtersize_fuzzy[i]/filtersize[i]
print("Average filter size factor: ", sum/len(filtersize_fuzzy))

# print factor of constructiontime
sum = 0
for i in range(len(constructiontime_fuzzy)):
    sum += constructiontime_fuzzy[i]/constructiontime[i]
print("Average construction factor: ", sum/len(constructiontime_fuzzy))

# print factor of querytime
sum = 0
for i in range(len(querytime_fp_fuzzy)):
    sum += querytime_fp_fuzzy[i]/querytime_fp[i]
print("Average query time nonmembers factor: ", sum/len(querytime_fp_fuzzy))
# print factor of querytime
sum = 0
for i in range(len(querytime_fn_fuzzy)):
    sum += querytime_fn_fuzzy[i]/querytime_fn[i]
print("Average query time members factor: ", sum/len(querytime_fn_fuzzy))

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Plot with markers and linewidth
plt.plot(size, constructiontime, label='BFF', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, constructiontime_fuzzy, label='fBFF',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Set Size n', fontsize=12)
plt.ylabel('Time (millisec.)', fontsize=12)
plt.title('Construction Time', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)
plt.savefig("../Plots/Constructiontime.png")

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Plot with markers and linewidth
plt.plot(size, filtersize, label='BFF', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, filtersize_fuzzy, label='fBFF',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Set Size n', fontsize=12)
plt.ylabel('Filter Size L', fontsize=12)
plt.title('Filter Size', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Filtersize.png")

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Plot with markers and linewidth
plt.plot(size, querytime_fn_fuzzy, label='Member Element - fBFF',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')
plt.plot(size, querytime_fp_fuzzy, label='Not Member Element - fBFF',  linestyle='-', linewidth=2, markersize=5, color='#d62728')
plt.plot(size, querytime_fn, label='Member Element - BFF', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, querytime_fp, label='Not Member Element - BFF', linestyle='-', linewidth=2, markersize=5, color='#2ca02c')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Set Size n', fontsize=12)
plt.ylabel('Querytime (millisec.)', fontsize=12)
plt.title('Querytime', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Querytime.png")

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Plot with markers and linewidth
plt.plot(size, querytime_fn, label='Member Element', linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, querytime_fp, label='Not Member Element', linestyle='-', linewidth=2, markersize=5, color='#2ca02c')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Set Size n', fontsize=12)
plt.ylabel('Querytime (millisec.)', fontsize=12)
plt.title('Querytime BFF', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Querytime_BFF.png")

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Plot with markers and linewidth
plt.plot(size, querytime_fn_fuzzy, label='Member Element',  linestyle='-', linewidth=2, markersize=5, color='#ff7f0e')
plt.plot(size, querytime_fp_fuzzy, label='Not Member Element',  linestyle='-', linewidth=2, markersize=5, color='#d62728')

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Set Size n', fontsize=12)
plt.ylabel('Querytime (millisec.)', fontsize=12)
plt.title('Querytime Fuzzy BFF', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/Querytime_fuzzy_BFF.png")

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Plot with markers and linewidth
plt.plot(size, fp_fuzzy, label='FP - fBFF',  linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, fn_fuzzy, label='FN - fBFF',  linestyle='-', linewidth=2, markersize=5, color='#2ca02c')
plt.plot(size, fp, label='FP - BFF', linestyle='--', linewidth=2, markersize=5, color='#1f77b4')

# Log scale for y-axis
#plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Set Size n', fontsize=12)
plt.ylabel('Number of FP/FN', fontsize=12)
plt.title('FP/FN Rates: BFF vs. fBFF', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/FP_FN.png")

# Create the figure and axis
plt.figure(figsize=(10, 6))

# Plot with markers and linewidth
plt.plot(size, fp_fuzzy, label='FP',  linestyle='-', linewidth=2, markersize=5, color='#1f77b4')
plt.plot(size, fn_fuzzy, label='FN',  linestyle='-', linewidth=2, markersize=5, color='#2ca02c')
plt.plot(size, fp_expected_fuzzy, label='FP expected', linestyle='--', linewidth=2, markersize=5, color='#1f77b4', alpha=0.5)
plt.plot(size, fn_expected_fuzzy, label='FN expected', linestyle='--', linewidth=2, markersize=5, color='#2ca02c', alpha=0.5)

# Log scale for y-axis
plt.yscale('log')

# Labels and title with improved font sizes
plt.xlabel('Set Size n', fontsize=12)
plt.ylabel('Number of FP/FN', fontsize=12)
plt.title('FP/FN Rates fBFF: Expected vs. Actual', fontsize=14, fontweight='bold')

# Add a legend with better positioning
plt.legend(loc='upper right', fontsize=11, frameon=True)

# Improve tick visibility
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)

# Add a light grid for better readability
plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.7)

plt.savefig("../Plots/FP_FN_fBFF.png")