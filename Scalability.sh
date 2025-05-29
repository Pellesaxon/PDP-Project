#!/bin/bash 

#SBATCH -M snowy
#SBATCH -A uppmax2025-3-7
#SBATCH -N 2
#SBATCH -p node
#SBATCH -n 25
#SBATCH --time=01:00:00
#SBATCH --error=job.strong.err
#SBATCH --output=job.strong.out

ml gcc/14.2.0 openmpi/5.0.5

processesArr=(1 4 9 16 25)
input=(8000)
          
runs_per_instance=5

# Loop through scenarios
for process in "${processesArr[@]}"; do
    times=()
    for i in $(seq 1 $runs_per_instance); do
        output=$(mpirun -np $process --bind-to none ./shearsort $input output_test.txt 2>&1)
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

    echo "Scenario=$input, Processes=$process, MedianRuntime=${median}s"
done
echo 
echo "All executions finished."