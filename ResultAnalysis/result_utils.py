import numpy as np
import sys
import os
def parse(v,rank_file,test_label_file):
    with open(rank_file) as r:
        rank_list=[line.rstrip('\n') for line in r]
    with open(test_label_file) as l:
        label_list=[line.rstrip('\n').split(',')[0] for line in l]
    rank_list=np.asarray(rank_list,np.float32)
    label_list=np.asarray(label_list,np.int)
    # compute the worst rank of each element as array cost
    u,v=np.unique(-rank_list,return_inverse=True)
    cost=(np.cumsum(np.bincount(v)))[v]
    ranks=[]
    for i in range(len(label_list)):
        if label_list[i]==1:
            ranks.append(cost[i])
    ranks=np.asarray(ranks,np.float32)
    if len(ranks)==0:
        return -1,-1
    min=ranks.min()
    avg=ranks.mean()
    return min,avg

def writetoLatexRQ(finalresult,LatextechsName,overallsubs):
    overallsubs.append('Overall')
    
    for s in range(len(overallsubs)):   # each subject and overall
        sys.stdout.write('\\multirow{4}{*}{'+overallsubs[s]+'}')
        techs=finalresult[s]
        for i in range(len(techs)):     # each model: multric, fluccs,trapt,rnn brnn...
            sys.stdout.write('&'+LatextechsName[i]+'&')
            lenth=len(techs[i])    # 5: top1,top3,top5,MFR,MAR

            for m in range(lenth-1):  #each value :top1,top3,top5,MFR
                if m<3:                      #top1,top3,top5
                    sys.stdout.write(str(techs[i][m])+'&')
                else:
                    value="%.2f" % techs[i][m]
                    sys.stdout.write(value+'&')
            sys.stdout.write("%.2f" % techs[i][lenth-1])  # MAR
            sys.stdout.write('\\\\')
            sys.stdout.write('\n')
            if i==2:
                sys.stdout.write('\cline{2-7}')

        sys.stdout.write('\\hline')
        
        sys.stdout.write('\n')

def delete_current_results():
    rootDir = './Rdata'
    for dirName, subdirList, fileList in os.walk(rootDir):
        for fname in fileList:
            os.remove(dirName + "/" + fname)