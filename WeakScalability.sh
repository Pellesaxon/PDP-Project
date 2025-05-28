#!/bin/bash 

#SBATCH -M snowy
#SBATCH -A uppmax2025-3-7
#SBATCH -p node
#SBATCH -n 16
#SBATCH --time=01:00:00
#SBATCH --error=job.weak.err
#SBATCH --output=job.weak.out

ml gcc/14.2.0 openmpi/5.0.5

processesArr=(1 4 9 16)
inputArr=(2000 4000 6000 8000)
          
runs_per_instance=5

length=${#processesArr[@]}

# Loop through scenarios
for (( i=0; i<length; i++ )); do
    input=${inputArr[$i]}
    process=${processesArr[$i]}

    times=()
    for (( run=1; run<=runs_per_instance; run++ )); do
        # Run quicksort and capture its time output
        output=$(mpirun -np $process --bind-to none ./shearsort $input output_test.txt 2>&1)
        # Extract the numeric time
        echo "${output}"
        runtime=$(echo "$output" | grep -oP 'TIME: \K[0-9]+\.[0-9]+' )
        times+=("$runtime")
        echo "${runtime}"
        echo "${times[@]}"
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

echo "All executions finished."