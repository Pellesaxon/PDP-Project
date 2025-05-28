#!/bin/bash 

#SBATCH -A uppmax2025-2-247
#SBATCH -p node
#SBATCH -n 16
#SBATCH --time=01:00:00
#SBATCH --error=job.strong.err
#SBATCH --output=job.strong.out

ml gcc openmpi

processes=(1 2 4 8 16)
input=("/proj/uppmax2025-2-247/nobackup/A3/inputs/input125000000.txt")  
#input=("input93.txt")
pivot_strategy=(1 2 3)
runs_per_instance=3

for process in "${processes[@]}"; do
    for pivot in "${pivot_strategy[@]}"; do
        times=()
        for i in $(seq 1 $runs_per_instance); do
            output=$(mpirun -np $process --bind-to none ./quicksort $input output_test.txt $pivot 2>&1)
            # Extract the numeric time after "TIME: "
            runtime=$(echo "$output" | grep -oP 'TIME: \K[0-9]+\.[0-9]+' )
            times+=("$runtime")
        done
        
        # Compute median inline
        sorted=($(printf "%s\n" "${times[@]}" | sort -n))
        n=${#sorted[@]}
        mid=$((n/2))
        if (( n % 2 == 1 )); then
            median=${sorted[$mid]}
        else
            a=${sorted[$((mid - 1))]}
            b=${sorted[$mid]}
            median=$(echo "($a + $b) / 2" | bc -l)
        fi

        echo "Scenario=$input, Processes=$process, PivotStrategy=$pivot, MedianRuntime=${median}s"
    done
    echo 
done
echo "All executions finished."