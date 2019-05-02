import result_analysis as result
from result_conf import *

def main():
	if RQ == "RQ1":
		dnns = ['DeepFL']
		models =  libsvm_models + dnns 
		result.RQ1(epoch_number,deep_data_dir,loss_function,models,subs,dnns)
	if RQ == "RQ2":
		result.RQ2()
	if RQ == "RQ3":
		result.RQ3()
	if RQ == "RQ4":
		result.RQ4()


#main function execution
if __name__=='__main__':
    main()