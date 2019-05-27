#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
from collections import defaultdict

def write_result(path,tech_list):
	result_map = defaultdict(list)
	for tech in tech_list:
		full_path = spec_and_mutation + tech + ".txt"
		with open(full_path) as f:
			for line in f:
				method_name = line.split(" ")[0]
				value = line.split(" ")[1].strip()
				result_map[method_name].append(float(value))
	with open(path,'a') as wf:
		for k,values in result_map.items():
			wf.write(k + " ")
			for v in values:
				wf.write(str(v) + ",")
			wf.write("\n")


spec_techs = ["Tarantula","Ochiai","Jaccard","Ample","RussellRao","Hamann","SrensenDice","Dice",
			   "Kulczynski1","Kulczynski2","SimpleMatching","Sokal","M1","M2","RogersTanimoto","Goodman",
				"Hamming","Euclid","Overlap","Anderberg","Ochiai2","Zoltar","Wong1","Wong2","Wong3","ER1a","ER1b",
				"ER5c","GP02","GP03","GP13","GP19","SBI","DStar"]
mut_type = ["type1","type2","type3","type4"]
mut_techs = spec_techs + ["Muse"]
mut_final_techs = []

for t in mut_techs:
	for ty in mut_type:
		mut_final_techs.append(t + ty)


root_path = sys.argv[1]
spec_and_mutation = root_path + "FinalFeatures/tempResult/Chart/1/"
write_path = root_path + "FinalFeatures/"

write_result(write_path + "/Spectrum/Chart/1.txt",spec_techs)
write_result(write_path + "/Mutation/Chart/1.txt",mut_final_techs)
	

