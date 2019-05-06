import result_analysis as result
from result_conf import *

def main():
	if RQ_number == "RQ1":
		dnns = ['dfl2']
		libsvm_models = ['Ochiai','MeOchiai','Multric','Fluccs','Trapt']		 
		model_name = ['Ochiai','Me-Ochiai','MULTRIC','FLUCCS','TraPT','MLP_DFL(2)']
		first_row = "Subjects,Techniques,Top-1,Top-3,Top-5,MFR,MAR"
		models =  libsvm_models + dnns
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,libsvm_models,model_name,RQ_number,first_row)
	if RQ_number == "RQ2":
		dnns = ['mlp','mlp2','birnn','dfl1','dfl2']
		libsvm_models = ['TraptJhawkByteIR']
		model_name = ['LIBSVM','MLP','MLP2','BiRNN','MLP_DFL(1)','MLP_DFL(2)']
		first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"
		models =  libsvm_models + dnns
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,libsvm_models,model_name,RQ_number,first_row)
	if RQ_number == "RQ2_2":
		techs = ['DeepFL','DeepFL-Spectrum','DeepFL-mut','DeepFL-metric','DeepFL-text']
		model_names = ['MLP_DFL(1)','MLP_DFL(1)-SpectrumInfor','MLP_DFL(1)-MutationInfor','MLP_DFL(1)-MetricsInfor',
						'MLP_DFL(1)-TextualInfo']
		n = len(techs)
		for i in range(n):
			tech = techs[i]
			model_name = model_names[i]
			dnns = ['dfl1']
			libsvm_models = []
			models =  libsvm_models + dnns
			first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR" 
			result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,libsvm_models,model_name,RQ_number,first_row)		
	if RQ_number == "RQ4":
		result.RQ4()

	

#main function execution
if __name__=='__main__':
    main()