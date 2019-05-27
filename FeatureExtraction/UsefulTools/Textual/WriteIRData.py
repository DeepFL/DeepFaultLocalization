import sys
import numpy as np


def readfile(Basedir,allquery):
    featuresize=len(allquery)
    featureDic={}
    for feature in allquery:
        Susfile=Basedir+feature+'.txt'
        with open(Susfile) as myfile:
            for line in myfile:
                items=line.split(" ")
                Method=items[0]
                value=items[1]
                index=allquery.index(feature)
                if Method not in featureDic:
                    allfeatures=np.zeros(featuresize)   ## initialize
                    allfeatures[index]=value
                    featureDic[Method]=allfeatures
                else:
                    featureDic[Method][index]=value
    

    return featureDic

def writefeatures(Dic,Wfile):
    with open(Wfile, "a") as myfile:
        for Method in Dic:
            myfile.write(Method+' ')
            values=Dic[Method]
            for v in values:
                myfile.write(str(v)+',')
            myfile.write('\n')

def main():
    project = sys.argv[1]
    ID = sys.argv[2]
    root_path = sys.argv[3]
    Susdir = root_path + "FinalFeatures/Textual/" + project + "/" + ID + "/tempResults/"
    Writefile = root_path + "FinalFeatures/Textual/" + project + "/" + ID +'.txt'
    IRquery = ['MtoT','MtoTc','MtoTFS','McltoT','McltoTc','McltoTFS','MmtoT','MmtoTc','MmtoTFS','MvtoT','MvtoTc','MvtoTFS',
			'McomtoT','McomtoTc','McomtoTFS']    # as 15 IR features
    
    FeatureDic=readfile(Susdir,IRquery)
    writefeatures(FeatureDic,Writefile)


main()
