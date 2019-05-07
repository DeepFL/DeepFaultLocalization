import result_analysis as result
from result_conf import *
import subprocess

def main():
	loss_function = 'softmax'
	tech = "DeepFL"
	first_row = "Techniques,Top-1,Top-3,Top-5,MFR,MAR"
	
	if RQ_number == "RQ1":		
		dnns = ['dfl2']
		libsvm_models = ['Ochiai','MeOchiai','Multric','Fluccs','Trapt']		 
		model_name = ['Ochiai','Me-Ochiai','MULTRIC','FLUCCS','TraPT','MLP_DFL(2)']
		first_row = "Subjects,Techniques,Top-1,Top-3,Top-5,MFR,MAR"
		models =  libsvm_models + dnns
		result_matrix = result.read_libsvm(subs,models,libsvm_models)
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
	
	if RQ_number == "RQ2":		
		dnns = ['mlp','mlp2','birnn','dfl1','dfl2']
		libsvm_models = ['TraptJhawkByteIR']
		model_name = ['LIBSVM','MLP','MLP2','BiRNN','MLP_DFL(1)','MLP_DFL(2)']
		models =  libsvm_models + dnns
		result_matrix = result.read_libsvm(subs,models,libsvm_models)
		result.RQ(epoch_number,deep_data_dir,loss_function,models,subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
	if RQ_number == "RQ2_2":		
		techs = ['DeepFL','DeepFL-Spectrum','DeepFL-Mutation','DeepFL-Metrics','DeepFL-Textual']
		m_names = ['MLP_DFL(1)','MLP_DFL(1)-SpectrumInfor','MLP_DFL(1)-MutationInfor','MLP_DFL(1)-MetricsInfor',
						'MLP_DFL(1)-TextualInfo']
		dnns = ['dfl1'] 
		
		n = len(techs)
		for idx in range(n):
			tech = techs[idx]
			model_name = [m_names[idx]]
			new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']
			result_matrix = result.initialize_result(subs,dnns) 								
			result.RQ(epoch_number,deep_data_dir,loss_function,models,new_subs,dnns,libsvm_models,model_name,RQ_number,first_row,tech)		
	
	if RQ_number == "RQ3":
		loss_funcs = ['softmax','epairwise']		
		dnns = ['dfl2']
		models = dnns
		model_name = ['MLP_DFL(2)']		
		for e in range(int(epoch_number)):
			e = e + 1
			for loss in loss_funcs:
				new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']
				result_matrix = result.initialize_result(subs,dnns) 
				result.RQ(e,deep_data_dir,loss,models,new_subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
		subprocess.call(['Rscript', 'RforRQ3.r', "loss_eval"])
  	
  	if RQ_number == "RQ4":
  		result.cross_vali_result()	 			
		techs = ['DeepFL','CrossDeepFL','CrossValidation']
		model_name = ['MLP_DFL(2)']   #not important
		dnns = ['dfl2']     
		models = dnns		
		for e in range(int(epoch_number)):
			e = e + 1
			for tech in techs:
				new_subs = ['Chart','Lang','Math','Time','Mockito','Closure']
				result_matrix = result.initialize_result(subs,dnns)
				result.RQ(e,deep_data_dir,loss_function,models,new_subs,dnns,model_name,RQ_number,first_row,tech,result_matrix)
  		subprocess.call(['Rscript', 'RforRQ3.r', "Cross"])
		

#main function execution
if __name__=='__main__':
    main()