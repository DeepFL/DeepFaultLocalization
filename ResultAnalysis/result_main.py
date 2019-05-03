import result_analysis as result
from result_conf import *

def main():
	if RQ_number == "RQ1":
		dnns = ['DeepFL']
		libsvm_models = ['Ochiai','MeOchiai','Multric','Fluccs','Trapt']		 
		model_name = ['Ochiai','Me-Ochiai','MULTRIC','FLUCCS','TraPT','MLP_DFL(2)']
		first_row = "Subjects,Techniques,Top-1,Top-3,Top-5,MFR,MAR"
	if RQ_number == "RQ2":
		dnns = ['mlp','mlp2','birnn','7-feature','DeepFL']
		libsvm_models = ['TraptJhawkByteIR']
		model_name = ['LIBSVM','MLP','MLP2','BiRNN','MLP_DFL(1)','MLP_DFL(2)']
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"
	if RQ_number == "RQ3":
		result.RQ3()
	if RQ_number == "RQ4":
		result.RQ4()

	models =  libsvm_models + dnns
	result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,libsvm_models,model_name,RQ_number,first_row)

#main function execution
if __name__=='__main__':
    main()