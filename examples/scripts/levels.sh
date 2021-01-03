TOP_DIR=/home/anderson/OptimizationCache
ALG_DIR=$TOP_DIR/examples/algorithms
BENCH_DIR=$TOP_DIR/benchmarks

GOAL=llvm_instructions
WEIGHT=1.0

export PYTHONPATH=$TOP_DIR
export LD_LIBRARY_PATH=$TOP_DIR/milepost_plugin/lib

python3 $ALG_DIR/levels.py \
	--goals $GOAL \
	--weights $WEIGHT \
	--benchmarks_directory $BENCH_DIR \
	--results_directory $TOP_DIR/examples/scripts/test_directory/levels \
	--benchmarks_filename $BENCH_DIR/reports/Angha_original_suite_toy.yaml