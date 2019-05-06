import result_analysis as result
from result_conf import *

def main():
	if RQ_number == "RQ1":
		dnns = ['dfl2']
		libsvm_models = ['Ochiai','MeOchiai','Multric','Fluccs','Trapt']		 
		model_name = ['Ochiai','Me-Ochiai','MULTRIC','FLUCCS','TraPT','MLP_DFL(2)']
		first_row = "Subjects,Techniques,Top-1,Top-3,Top-5,MFR,MAR"
	if RQ_number == "RQ2":
		dnns = ['mlp','mlp2','birnn','dfl1','dfl2']
		libsvm_models = ['TraptJhawkByteIR']
		model_name = ['LIBSVM','MLP','MLP2','BiRNN','MLP_DFL(1)','MLP_DFL(2)']
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"
	if RQ_number == "RQ2_2":
		dnns = ['7-feature','7-selection-spec','4-selection-mut','7-selection-complex','7-selection-similar']
		libsvm_models = []
		model_name = ['MLP_DFL(1)','MLP_DFL(1)-SpectrumInfor','MLP_DFL(1)-MutationInfor','MLP_DFL(1)-MetricsInfor',
						'MLP_DFL(1)-TextualInfo']
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR" 
	if RQ_number == "RQ4":
		result.RQ4()

	models =  libsvm_models + dnns
	result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,libsvm_models,model_name,RQ_number,first_row)

#main function execution
if __name__=='__main__':
    main()