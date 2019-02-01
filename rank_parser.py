import os
import time
import numpy as np
from collections import OrderedDict
import sys
import decimal
import sys
DeepDataDir=sys.argv[1]

tech = sys.argv[3]     # Tech names such as "DeepFL", "DeepFL-Spectrum"...
model = sys.argv[4]    # model name, such as "mlp" "mlp2" "rnn" "birnn"  
loss = sys.argv[5]     # loss function   

def initializeResult(allsub,tvector):
    # has 7 elements:6 project + 1 overall
    resultMatrix=[] 
    Overall=len(allsub)+1 
    
    #each element has several model: trapt...rnn,birnn,mlp,mlp2
    modelsize=len(tvector) 
    
    for i in range(Overall):
        modelvector=[]
        for m in range(modelsize):
            a=[0,0,0,0.00,0.00]            #top1 top3, top5, mfr, mar
            array=np.array(a,dtype=object)           
            modelvector.append(array)
        resultMatrix.append(modelvector)   #initialize result matrix 
    return resultMatrix

def parse(rank_file,test_label_file):
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


def readDeepResult(dir,subs,tech,dnns,epoch,vers,resultBysub,techsvector,ResultDir):
    # subs: chart,lang,time...  
    # tech: spectrumTestJahwkByte
    # dnns: mlp mlp2 rnn... 
    
    out_dir=ResultDir 
    train_file='Train.csv'
    train_label_file='TrainLabel.csv'
    test_file='Test.csv'
    test_label_file='TestLabel.csv'
    
    test_name_file='list.txt'

    truevers=[] 
    for v in range(len(subs)):  # real version numbers of each subject
        truevers.append(0)
    
    # iterate all projects                                                                               
    for s in range(len(subs)):
        sub=subs[s]
        ver=vers[s]
        #print(sub)
        for d in range(len(dnns)):
          if dnns[d]=='fc':   # only print birnn
            tops=np.zeros(4)
            ranks=np.zeros(2)
            actual_ver=0
            # iterate all versions of the current proj                                               
            for v in range(ver):
                v=str(v+1)
                test_label_path=dir+tech+'/'+sub+'/'+v+'/'+test_label_file
                susp_path=out_dir+'/'+sub+'/'+v+'/'+tech+'/'+dnns[d]+'-'+loss+'-'+str(epoch)
                min,avg=parse(susp_path,test_label_path)
                if min == -1:
                    continue
                if min <= 1:
                    tops[0]+=1
                if min <=3:
                    tops[1]+=1
                if min<=5:
                    tops[2]+=1
                if min<=10:
                    tops[3]+=1
                ranks[0]+=min
                ranks[1]+=avg
                actual_ver+=1
            
            ranks=ranks/actual_ver    
            print(sub,tech, epoch,dnns[d], tops,ranks)      # print each result
            truevers[s]=actual_ver
            result=(int(tops[0]),
                    int(tops[1]),
                    int(tops[2]),
                    round(float(ranks[0]),2),
                    round(float(ranks[1]),2))
            result=np.array(result, dtype=object)
                
            subindex=s
            modelindex=techsvector.index(dnns[d])
            resultBysub[subindex][modelindex]=result
    return truevers

def CalculateOverall(resultBysub,truevers,techsvector):
    verssum=0
    for v in truevers:
        verssum=verssum+v
    
    for s in range(len(resultBysub)-1):                  #Loop Time,Chart,Math,Lang,Closure
        for m in range(len(techsvector)):                #loop multri,flucss, trapt, rnn....
            resultBysub[len(resultBysub)-1][m][0]=resultBysub[len(resultBysub)-1][m][0]+resultBysub[s][m][0]
            resultBysub[len(resultBysub)-1][m][1]=resultBysub[len(resultBysub)-1][m][1]+resultBysub[s][m][1]
            resultBysub[len(resultBysub)-1][m][2]=resultBysub[len(resultBysub)-1][m][2]+resultBysub[s][m][2]
            resultBysub[len(resultBysub)-1][m][3]=resultBysub[len(resultBysub)-1][m][3]+resultBysub[s][m][3]*truevers[s]
            resultBysub[len(resultBysub)-1][m][4]=resultBysub[len(resultBysub)-1][m][4]+resultBysub[s][m][4]*truevers[s]
            
    
    for m in range(len(techsvector)):
        resultBysub[len(resultBysub)-1][m][3]=round(resultBysub[len(resultBysub)-1][m][3]/verssum,2)
        resultBysub[len(resultBysub)-1][m][4]=round(resultBysub[len(resultBysub)-1][m][4]/verssum,2) 



def main():  
    epochnumber = sys.argv[6]   # epoch number
    sub = sys.argv[7]
    ResultDir = sys.argv[2]   #directory of results
    subs=[]
    vers= []
    if(sub == 'all'):
        subs=['Chart','Lang','Math','Time','Closure','Mockito']
        vers=[26, 65, 106, 27, 133, 38]
    elif(sub=='noClosure'):
        subs=['Chart','Lang','Math','Time','Mockito']
        vers=[26, 65, 106, 27, 38] 
    elif(sub=='Chart'):
        subs=['Chart']   
        vers=[26]
    elif(sub=='Mockito'):
        subs=['Mockito']
        vers=[38]
    else:
        print('WRONG!'+sub)
        exit()

    #dnns=['mlp','mlp2','rnn','birnn','fc']
    #techsvector=[dnns[0],dnns[1],dnns[2],dnns[3],dnns[4]]
    dnns=['fc']
    techsvector=[dnns[0]]
    
    

    #For RQ1,  current 3 libsvm and 4 deeplearning(SpectrumTestJawkByte)
    resultBysub=initializeResult(subs,techsvector)
    
    #read deep learning result and RQ2 line grpah trend compare mlp,mlp2,rnn,birnn
    truevers=readDeepResult(DeepDataDir,subs,tech,dnns,epochnumber,vers,resultBysub,techsvector,ResultDir)
    
    #Calculate overallresult
    CalculateOverall(resultBysub,truevers,techsvector)
    
    m = dnns.index(model)
    print("        Top-1   Top-3   Top-5   MFR     MAR")
    for sub in range(0,6):   # chart,time...
        sys.stdout.write(subs[sub]+"\t")
        for metric in range(0,5):   # top1 top2...mar
            sys.stdout.write(str(resultBysub[sub][m][metric])+"\t")
        print('')
    sys.stdout.write("Overall" + "\t")
    for metric in range(0,5):   # top1 top2...mar
        sys.stdout.write(str(resultBysub[len(subs)][m][metric])+"\t")
    print('')
if __name__ == '__main__':
  main()



