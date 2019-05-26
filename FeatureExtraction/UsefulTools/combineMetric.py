import sys
from collections import defaultdict
root_path = sys.argv[1]

source_result = root_path + "/FeatureValues/Complexity/Chart/1source.txt"
byte_result = root_path + "/FeatureValues/Complexity/Chart/1byte.txt"

source_result_map = defaultdict(str)
comb_result_map = defaultdict(str)

with open(source_result) as sr:
	for line in sr:
		method_name = line.split(" ")[0]
		vs = line.split(" ")[1].strip()
		source_result_map[method_name] = vs
with open(byte_result) as br:
	for line in br:
		method_name = line.split(" ")[0]
		vs = line.split(" ")[1].strip()
		if method_name in comb_result_map:
			comb_result_map[method_name] = source_result_map[method_name] + vs

comb_result_path = root_path + "/FeatureValues/Complexity/Chart/1.txt"
with open(comb_result_path,'a') as cr:
	for m in comb_result_map:
		cr.write(m + " ")
		cr.write(comb_result_map[m])
		cr.write("\n")