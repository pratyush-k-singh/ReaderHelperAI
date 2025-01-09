#!/usr/bin/env python3

import subprocess
import time
import json
import sys
import os
from pathlib import Path
import argparse
import numpy as np
import matplotlib.pyplot as plt

def run_benchmark(executable_path: str, query: str, iterations: int = 100) -> dict:
    """Run performance benchmarks on the recommender system."""
    results = {
        'query_times': [],
        'memory_usage': [],
        'groq_latency': []
    }
    
    for i in range(iterations):
        start_time = time.time()
        
        # Run the recommender with timing
        process = subprocess.Popen(
            [executable_path, '--benchmark', query],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        stdout, stderr = process.communicate()
        
        # Parse the output
        try:
            output = json.loads(stdout)
            results['query_times'].append(output['query_time'])
            results['memory_usage'].append(output['memory_usage'])
            results['groq_latency'].append(output['groq_latency'])
        except json.JSONDecodeError:
            print(f"Error parsing output on iteration {i}")
            continue
            
    return results

def analyze_results(results: dict) -> dict:
    """Analyze benchmark results."""
    analysis = {}
    
    for metric, values in results.items():
        analysis[metric] = {
            'mean': np.mean(values),
            'std': np.std(values),
            'min': np.min(values),
            'max': np.max(values),
            'p95': np.percentile(values, 95),
            'p99': np.percentile(values, 99)
        }
    
    return analysis

def plot_results(results: dict, output_dir: Path):
    """Generate plots for benchmark results."""
    for metric, values in results.items():
        plt.figure(figsize=(10, 6))
        plt.hist(values, bins=30)
        plt.title(f'{metric} Distribution')
        plt.xlabel('Time (ms)')
        plt.ylabel('Frequency')
        plt.savefig(output_dir / f'{metric}_distribution.png')
        plt.close()
        
        # Create percentile plot
        plt.figure(figsize=(10, 6))
        percentiles = np.percentile(values, range(1, 101))
        plt.plot(range(1, 101), percentiles)
        plt.title(f'{metric} Percentiles')
        plt.xlabel('Percentile')
        plt.ylabel('Time (ms)')
        plt.grid(True)
        plt.savefig(output_dir / f'{metric}_percentiles.png')
        plt.close()

def main():
    parser = argparse.ArgumentParser(description='Benchmark the Book Recommender system')
    parser.add_argument('--executable', required=True, help='Path to the recommender executable')
    parser.add_argument('--query', default='fantasy books with magic', help='Query to benchmark')
    parser.add_argument('--iterations', type=int, default=100, help='Number of iterations')
    parser.add_argument('--output-dir', default='benchmark_results', help='Output directory for results')
    
    args = parser.parse_args()
    
    # Create output directory
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)
    
    print(f"Running benchmark with query: '{args.query}'")
    print(f"Iterations: {args.iterations}")
    
    # Run benchmarks
    results = run_benchmark(args.executable, args.query, args.iterations)
    
    # Analyze results
    analysis = analyze_results(results)
    
    # Save results
    with open(output_dir / 'benchmark_results.json', 'w') as f:
        json.dump({
            'config': vars(args),
            'results': results,
            'analysis': analysis
        }, f, indent=2)
    
    # Generate plots
    plot_results(results, output_dir)
    
    # Print summary
    print("\nBenchmark Results:")
    print("=" * 50)
    for metric, stats in analysis.items():
        print(f"\n{metric}:")
        print(f"  Mean: {stats['mean']:.2f} ms")
        print(f"  Std Dev: {stats['std']:.2f} ms")
        print(f"  95th percentile: {stats['p95']:.2f} ms")
        print(f"  99th percentile: {stats['p99']:.2f} ms")
        print(f"  Min: {stats['min']:.2f} ms")
        print(f"  Max: {stats['max']:.2f} ms")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\nBenchmark interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"Error during benchmark: {e}")
        sys.exit(1)
        