import sys
from collections import defaultdict
root_path = sys.argv[1]

source_result = root_path + "/FeatureValues/Complexity/Chart/1.txt"
byte_result = root_path + "/FeatureValues/Complexity/Chart/1byte.txt"

source_result_map = defaultdict(str)
comb_result_map = defaultdict(str)

with open(source_result) as sr:
	for line in sr:
		method_name = line.split(" ")[0]
		vs = line.split(" ")[1].strip()
		source_result_map[method_nam] = vs
print(source_result_map)		
