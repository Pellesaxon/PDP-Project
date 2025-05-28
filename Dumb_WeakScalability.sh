#!/bin/bash 

#SBATCH -M snowy
#SBATCH -A uppmax2025-3-7
#SBATCH -p node
#SBATCH -n 16
#SBATCH --time=01:00:00
#SBATCH --error=job.weak.err
#SBATCH --output=job.weak.out

ml gcc openmpi

processesArr=(1 4 9 16)
inputArr=(2000 4000 6000 8000)
          
runs_per_instance=5

length=${#processesArr[@]}

# Loop through scenarios
for (( i=0; i<length; i++ )); do
    input=${inputArr[$i]}
    process=${processesArr[$i]}

    echo "Scenario=$input, Processes=$process"
    for (( run=1; run<=runs_per_instance; run++ )); do
        # Run quicksort and capture its time output
        mpirun -np $process --bind-to none ./shearsort $input output_test.txt
    done
done

echo "All executions finished."