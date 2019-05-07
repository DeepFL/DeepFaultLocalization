import os
import time
import numpy as np
from collections import OrderedDict
import sys
import decimal
import sys
from rank_parser import *
import matplotlib.pyplot as plt

def main():  
    DeepDataDir=sys.argv[1]

    tech = sys.argv[3]     # Tech names such as "DeepFL", "DeepFL-Spectrum"...
    model = sys.argv[4]    # model name, such as "mlp" "mlp2" "rnn" "birnn"  
    loss = sys.argv[5]     # loss function   
    epochnumber = int(sys.argv[6])   # epoch number (notice that this is a maximal number)
    sub = 'all'
    dump_step = int(sys.argv[8]) 
    figure_name = sys.argv[9]
    ResultDir = sys.argv[2] + '/'+figure_name   #directory of results
    subs=[]
    vers= []
    if(sub == 'all'):
        subs=['Chart','Lang','Math','Time','Closure','Mockito']
        vers=[26, 65, 106, 27, 133, 38]
    # elif(sub=='10fold'):
    #     subs=['10fold']
    #     vers=[10]
    else:
        print('WRONG!'+sub)
        exit()

    
    dnns=['mlp','mlp2','rnn','df1','df2']
    #techsvector=[dnns[0],dnns[1],dnns[2],dnns[3],dnns[4]]
    #dnns=['fc']
    techsvector=[dnns[0]]
    final_result = []
    for i in range(epochnumber/dump_step):
        step = (i+1)*dump_step
        #For RQ1,  current 3 libsvm and 4 deeplearning(SpectrumTestJawkByte)
        resultBysub=initializeResult(subs,techsvector)
        #read deep learning result and RQ2 line grpah trend compare mlp,mlp2,rnn,birnn
        truevers=readDeepResult(DeepDataDir,subs,tech,dnns,step,vers,resultBysub,techsvector,ResultDir)
        #Calculate overallresult
        CalculateOverall(resultBysub,truevers,techsvector)
        final_result.append(resultBysub)
    final_result = np.array(final_result)
    m = dnns.index(model)
    plt.figure(figsize=(8,14))
    for i in range(len(subs)):
        ax1 = plt.subplot(int((len(subs)+1)/2)+1,2,i+1)
        x = [(j+1)*dump_step for j in range(epochnumber/dump_step)]
        ax1.plot(x,final_result[:,i,m,0],marker='o',label='top1',markersize=2)
        ax1.plot(x,final_result[:,i,m,1],marker='*',label='top3',markersize=2)
        ax1.plot(x,final_result[:,i,m,2],marker='d',label='top5',markersize=2)
        ax1.set_xlabel('steps')
        ax2 = ax1.twinx()
        ax2.plot(x,final_result[:,i,m,3],marker='s',label='MFR',color='black',markersize=2)
        ax2.plot(x,final_result[:,i,m,4],marker='.',label='MAR',color = 'yellow',markersize=2)
        plt.title(subs[i],loc='right')

    #PLOT ALL subfigure which including MLP and RNN comparison
    ax1 = plt.subplot(int((len(subs)+1)/2)+1,1,int(len(subs)/2)+1)
    #x = [(j+1)*dump_step for j in range(epochnumber/dump_step)] + ['MLP','RNN']
    x = np.array([(j+1)*dump_step for j in range(epochnumber/dump_step)])
    #top1_y = np.concatenate((final_result[:,len(subs),m,0],[187,215]))
    print(np.shape(final_result[:,len(subs),m,0]))
    top1_y = final_result[:,len(subs),m,0]
    top3_y = final_result[:,len(subs),m,1]
    top5_y = final_result[:,len(subs),m,2]
    print("sum top1 y is",top1_y)
    print("sum top3 y is",top3_y)
    print("sum top5 y is",top5_y)
    max_y = max(top1_y)
    second_y = max(top1_y[:-1])
    max_x = x[np.argmax(top1_y)]
    second_x = x[np.argmax(top1_y[:-1])]
    print(np.argmax(second_y),second_x,second_y)
    ax1.plot(x,top1_y,marker='o',label='top1',markersize=2)
    #ax1.annotate(str(max_x)+","+str(max_y),xy=(max_x,max_y),color='red')
    #ax1.annotate(str(second_x)+","+str(second_y),xy=(str(second_x),second_y),color='blue')
    #ax1.plot(x,np.concatenate((final_result[:,len(subs),m,1],[271,282])),marker='*',label='top3',markersize=2)
    #ax1.plot(x,np.concatenate((final_result[:,len(subs),m,2],[305,304])),marker='d',label='top5',markersize=2)
    ax1.plot(x,final_result[:,len(subs),m,1],marker='*',label='top3',markersize=2)
    ax1.plot(x,final_result[:,len(subs),m,2],marker='d',label='top5',markersize=2)
    ax1.set_xlabel('steps')
    ax2 = ax1.twinx()
    ax2.plot(x,final_result[:,len(subs),m,3],label='MFR',marker='s',color='black',markersize=2)
    ax2.plot(x,final_result[:,len(subs),m,4],label='MAR',marker='.',color = 'yellow',markersize=2)
    #ax2.plot(x,np.concatenate((final_result[:,len(subs),m,3],[8.27,6.62])),label='MFR',marker='s',color='black',markersize=2)
    #ax2.plot(x,np.concatenate((final_result[:,len(subs),m,4],[10.67,8.54])),label='MAR',marker='.',color = 'yellow',markersize=2)
    plt.title('ALL',loc='right')
    plt.xlabel('steps')
    ax1.legend(loc='upper right')
    ax2.legend(loc='upper left')
    plt.subplots_adjust(wspace =0.3, hspace =0.3)
    #plt.show()
    plt.savefig("fig_result/"+figure_name)
main()



