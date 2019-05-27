import os
import time
import numpy as np
from collections import OrderedDict
import sys
import decimal
import sys
import result_utils as ut
from result_conf import *
import shutil

def initialize_result(allsub,tvector):
    result_matrix = [] 
    overall = len(allsub) + 1     
    #each element has several model: trapt...mlp,mlp2..MLP_variant
    model_size = len(tvector) 
    
    for i in range(overall):
        model_vector = []
        for m in range(model_size):
            a = [0,0,0,0.00,0.00]            #top1 top3, top5, mfr, mar
            array = np.array(a,dtype=object)           
            model_vector.append(array)
        result_matrix.append(model_vector)   #initialize result matrix 
    return result_matrix

def read_libsvm_result(finalresult,libsvmResultPath,allsub,techvector):
    techname=libsvmResultPath[libsvmResultPath.rfind("/")+1:libsvmResultPath.index('.txt')]
    techindex=techvector.index(techname)
    with open(libsvmResultPath) as f:
        lines=f.readlines()
        for i in range(0, len(lines),2):
            sub=lines[i].strip()
            subject=sub[0:sub.index('.')]
            subindex=allsub.index(subject)
            
            rank=lines[i+1].strip().split(',')
            array=[]
            count=0
            for r in rank:
                if r!='':
                    value=float(r)
                    if count<3:
                        array.append(int(value))
                    else:
                        array.append(round(value,2))
                count=count+1
            finalresult[subindex][techindex]=array


def read_deep_result(dir,subs,tech,dnns,epoch,vers,resultBysub,techsvector,loss):
    # subs: chart,lang,time...  
    # tech: spectrumTestJahwkByte
    # dnns: mlp mlp2 rnn... 
    
    out_dir=result_dir 
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
            tops=np.zeros(4)
            ranks=np.zeros(2)
            ranks_min = list()  #first
            ranks_avg = list()  #all
            actual_ver=0
            # iterate all versions of the current proj                                               
            for v in range(ver):
                v=str(v+1)
                test_label_path = dir + tech + '/' + sub + '/' + v + '/' + test_label_file
                susp_path = os.path.join(out_dir,sub,v,tech,dnns[d]+'-'+loss+'-'+str(epoch))
                #susp_path = out_dir + '/result/' + dnns[d] + '/' + sub + '/' + v + '/' + dnns[d] + '/fc-' + loss_function + '-' + str(epoch)
                min,avg=ut.parse(v,susp_path,test_label_path)
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
                ranks_min.append(min)
                ranks_avg.append(avg)
                actual_ver+=1
            
            ranks=ranks/actual_ver 
            ranks_min = np.array(ranks_min)
            ranks_avg = np.array(ranks_avg)
            ranks_min_median = np.median(ranks_min)
            ranks_avg_median = np.median(ranks_avg)
            #print(sub,tech, epoch,dnns[d], tops,ranks)      # print each result
            truevers[s]=actual_ver
            result=(int(tops[0]),
                    int(tops[1]),
                    int(tops[2]),
                    round(float(ranks[0]),2),
                    round(float(ranks[1]),2),)
                   # round(float(ranks_min_median),2),
                    #round(float(ranks_avg_median),2))
            result=np.array(result, dtype=object)
                
            subindex=s
            modelindex=techsvector.index(dnns[d])
            resultBysub[subindex][modelindex]=result  
    return truevers

def get_overall(resultBysub,truevers,techsvector,subsize):
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

def write_to_csv_RQ(final_result,LatextechsName,overallsubs,RQ_number,first_row):
    overallsubs.append('Overall')
    with open(RQ_number + ".csv","a") as RQ:
        RQ.write(first_row)
        RQ.write("\n")
        for s in range(len(overallsubs)):   # each subject and overall
            if (RQ_number == "RQ2" or RQ_number == "RQ2_2") and s < len(overallsubs) - 1:
                continue
            
            techs = final_result[s]

            for i in range(len(techs)):     # each model: multric, fluccs,trapt,rnn brnn...
                lenth = len(techs[i])    # 5: top1,top3,top5,MFR,MAR
                
                if RQ_number == "RQ1":
                    if i == 3:
                        RQ.write(overallsubs[s] + ',' + LatextechsName[i] + ',')
                    else:
                        RQ.write(',' + LatextechsName[i] + ',')
                else:
                     RQ.write(LatextechsName[i] + ',')
                for m in range(lenth - 1):  #each value :top1,top3,top5,MFR
                    if m < 3:                      #top1,top3,top5
                        RQ.write(str(techs[i][m])+',')
                    else:
                        value = "%.2f" % techs[i][m]
                        RQ.write(value + ',')
                RQ.write("%.2f" % techs[i][lenth-1])  # MAR
                RQ.write('\n')
            RQ.write('\n')
        


def write_to_R(result_matrix,subs,loss_or_tech,dnns):
    subs.append("Overall")
    #print(len(RQ2TrendData[0]))
    for s in range(len(subs)):
        for d in range(len(dnns)):
            if not os.path.exists('Rdata/' + loss_or_tech +'/'):
                os.makedirs('Rdata/' + loss_or_tech +'/')
            writefile = 'Rdata/' + loss_or_tech +'/' + subs[s] + '_dfl2.txt'
            with open(writefile, "a") as myfile:
                myfile.write(str(result_matrix[s][d][0])+','+
                             str(result_matrix[s][d][1])+','+
                             str(result_matrix[s][d][2])+','+
                             str(result_matrix[s][d][3])+','+
                             str(result_matrix[s][d][4])) 
                myfile.write('\n')        

def read_libsvm(subs,techs_vector,libsvm_models):
    result_matrix = initialize_result(subs,techs_vector)    
    #read libsvm
    for i in range(0,len(libsvm_models)):   
        libsvm_result_path = 'LIBSVMResult/' + techs_vector[i] + '.txt'
        read_libsvm_result(result_matrix,libsvm_result_path,subs,techs_vector)
    return result_matrix

def RQ(epoch_number,deep_data_dir,loss,techs_vector,subs,dnns,model_name,RQ,first_row,tech,result_matrix):   
    true_vers = read_deep_result(deep_data_dir,subs,tech,dnns,epoch_number,vers,result_matrix,techs_vector,loss)
    #Calculate overallresult
    get_overall(result_matrix,true_vers,techs_vector,len(subs))
    if RQ == "RQ1" or RQ == "RQ2" or RQ == "RQ2_2":
    	write_to_csv_RQ(result_matrix,model_name,subs,RQ,first_row)
    elif RQ == "RQ3":
    	write_to_R(result_matrix,subs,loss,dnns)
    elif RQ == "RQ4":
        write_to_R(result_matrix,subs,tech,dnns)
def delete_exsisting():
    for (root,dirs,files) in os.walk(result_dir): 
        if "CrossValidation" in root and "10fold" not in root:
            shutil.rmtree(root)
    #for p in subs:
    #    full_path = deep_data_dir + "CrossValidation/" + p
    #    if os.path.isdir(full_path):
    #        shutil.rmtree(full_path)

def cross_vali_result():
    resultRoot = result_dir + '/10fold/'
    labelpath = deep_data_dir + 'CrossValidation/10fold/'
    delete_exsisting()
    
    vers = 10
    for v in range(1,vers + 1):
        for ep in range(1, int(epoch_number) + 1, 1):
            rank_file = os.path.join(resultRoot,str(v),'CrossValidation','dfl2-softmax-' + str(ep))
            test_label_file = os.path.join(labelpath, str(v),'TestLabel.csv')
            PV_file = os.path.join(labelpath,str(v),'PandV.txt')
            with open(rank_file) as r:
                rank_list=[line.rstrip('\n') for line in r]
            with open(test_label_file) as l:
                label_list=[line.rstrip('\n') for line in l]
            with open(PV_file) as r:
                PV_list=[line.rstrip('\n') for line in r]
            DataLength = len(rank_list)
            for l in range(0, DataLength):
                project = PV_list[l].split('-')[0]
                version = PV_list[l].split('-')[1].strip()
                newRespath = os.path.join(result_dir,project,version,'CrossValidation')
                newLabel = os.path.join(deep_data_dir,'CrossValidation',project,version)
                if not os.path.exists(newRespath):
                    os.makedirs(newRespath)

                if not os.path.exists(newLabel):
                    os.makedirs(newLabel)

                result_file = newRespath + '/dfl2-softmax-' + str(ep)
                with open(result_file,'a') as wd:
                    wd.write(rank_list[l])
                    wd.write('\n')
                
                #if ep == 1:
                #    lable_file = newLabel + '/TestLabel.csv'
                #    with open(lable_file,'a') as wd:
                #        wd.write(label_list[l])
                #        wd.write('\n')
        


def cross_vali_result_with_result():
    #resultRoot = result_dir + '/10fold/'
    labelpath = deep_data_dir + 'CrossValidation/10fold/'
    #delete_exsisting()
    
    vers = 10
    for v in range(1,vers + 1):
        for ep in range(1, int(epoch_number) + 1, 1):
            #rank_file = os.path.join(resultRoot,str(v),'CrossValidation','dfl2-softmax-' + str(ep))
            test_label_file = os.path.join(labelpath, str(v),'TestLabel.csv')
            PV_file = os.path.join(labelpath,str(v),'PandV.txt')
            #with open(rank_file) as r:
                #rank_list=[line.rstrip('\n') for line in r]
            with open(test_label_file) as l:
                label_list=[line.rstrip('\n') for line in l]
            with open(PV_file) as r:
                PV_list=[line.rstrip('\n') for line in r]
            DataLength = len(PV_list)
            for l in range(0, DataLength):
                project = PV_list[l].split('-')[0]
                version = PV_list[l].split('-')[1].strip()
                #newRespath = os.path.join(result_dir,project,version,'CrossValidation')
                newLabel = os.path.join(deep_data_dir,'CrossValidation',project,version)
                #if not os.path.exists(newRespath):
                    #os.makedirs(newRespath)

                if not os.path.exists(newLabel):
                    os.makedirs(newLabel)

                #result_file = newRespath + '/dfl2-softmax-' + str(ep)
                #with open(result_file,'a') as wd:
                    #wd.write(rank_list[l])
                    #wd.write('\n')
                
                if ep == 1:
                    lable_file = newLabel + '/TestLabel.csv'
                    with open(lable_file,'a') as wd:
                        wd.write(label_list[l])
                        wd.write('\n')

